#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>

#include "shellmemory.h"
#include "interpreter.h"

struct PCB{
    int PID;
    int length;
    int counter;  // line of cur instruction
    int frameIndex;
    int lineIndex;
    int *frame_ptr;  // pointer to page frame
    int frame_size;
    int score;
    char* script;
    struct PCB *next;
};

struct PCB *head = NULL;  // ready queue
int counter = 0;
int uniquePID = 1;


void createPCB(int pageFrame[], int pageFrame_size, int length, int isSort, char *script){
    // sort = 0 : will add new pcb to the end of the queue
    // sort = 1 : will add pcb in increasing order

    struct PCB *pcb = malloc(sizeof(struct PCB));

    pcb->PID = uniquePID;
    uniquePID++;
    pcb->length = length;
    pcb->counter = 0;
    pcb->frameIndex = 0;
    pcb->lineIndex = 0;

    pcb->frame_ptr = malloc(pageFrame_size*sizeof(int));
    for(int i = 0; i < pageFrame_size; i++){
        (pcb->frame_ptr)[i] = pageFrame[i];
    }

    pcb->frame_size = pageFrame_size;
    pcb->score = length;
    pcb->script = malloc(100*sizeof(char));
    int j = 0;
    while(script[j] != '\0'){
        (pcb->script)[j] = script[j];
        j++;
    }
    (pcb->script)[j] = '\0';
    pcb->next = NULL;

    if(head == NULL){
         head = pcb;
     } else if(isSort == 0){
         // add pcb to the end of queue
         struct PCB *current = head;
         while(current->next != NULL){
             current = current->next;
         }
         current->next=pcb;

     } else if(isSort == 1){
         // add pcb in increasing order(length)
         struct PCB *current = head;
         struct PCB *prev = NULL;
          while(1){
             if(current->length > pcb->length){ 
                 pcb->next = current;
                 if(prev == NULL){
                     head = pcb;
                 } else {
                     prev->next = pcb;
                 }
                 break;
             } else if(current->next == NULL){
                 current->next = pcb;
                 break;
             }
             prev = current;
             current = current->next;
         }
     }  
}

void clearShellMemory(struct PCB *cur){
    // remove source code from the Shell memory
    for(int i = 0; i< cur->frame_size; i++){
        if((cur->frame_ptr)[i] != -1){
            int index = ((cur->frame_ptr)[i]) * 3;
            for(int j = 0; j < 3; j++){
                mem_reset_index(index);
                index ++;
            }
        }
    }
}

void clearPageFrame(){
    for(int i = 0; i < FRAMESIZE; i++){
         mem_reset_index(i);
    }
}

void updatePageFrame(struct PCB* load, char* fileName, int evictedPageFrame){
    int index = evictedPageFrame * 3;
	for(int j = 0; j < 3; j++){
		mem_reset_index(index);
		index++;
	}
     struct PCB *cur = head;
     while(cur != NULL){
         if(strcmp(fileName, cur->script)==0){
             for(int i = 0; i < cur->frame_size; i++){
                 if((cur->frame_ptr)[i] == evictedPageFrame){
                     (cur->frame_ptr)[i] = -1;
                 }
             }
         }
         cur = cur->next;
     }
     loadSinglePage(load->script, (load->counter)-(load->lineIndex));
 }


int runProgram(struct PCB *cur, int size){
    // return 0 if program reach the end
    // return 1 if program is unfinish

    // run from where it is left off to specify size
    int c = 0;
    int isFinish = 0;

    // run for "size" instructions or until the end of program
    while(c < size && (cur->counter < cur->length)){
       
        if(cur->frame_ptr[cur->frameIndex] == -1){
            // page fault, need to load one more page

            int frameIndex = loadSinglePage(cur->script, (cur->counter)-(cur->lineIndex));
           
            if(frameIndex == -1){
                // fail to find empty spot
			    // evict least recently used page
			    // call function in shellmemory to evict least recenly used page
		    	// and retry evict
                int evictedPageFrame = 0;
                char* fileName = evictLeastRecentlyUsed(&evictedPageFrame);  
                printPageFault(evictedPageFrame);
                updatePageFrame(cur, fileName, evictedPageFrame);
                updateRecentlyUsed(evictedPageFrame);
                cur->frame_ptr[cur->frameIndex] = evictedPageFrame;
            } else {
                updateRecentlyUsed(frameIndex);
                cur->frame_ptr[cur->frameIndex] =  frameIndex;
            }
            return 1;
        }

        runIndex(cur->frame_ptr[cur->frameIndex], cur->lineIndex);

        (cur->lineIndex)++;
            
        if(cur->lineIndex == 3){
            // finish the 3 lines or cur frame
            cur->lineIndex = 0;
            (cur->frameIndex)++;
        }
        c++;
        (cur->counter)++;
    }
    
  
    if((cur->counter) >= (cur->length)){
        return 0;
    } else {
        return 1;
    }
}

void FCFSandSJF(){
    struct PCB *del = NULL;

    while(head != NULL){
        int isFinish = runProgram(head, head->length);

        if(isFinish == 0){
            del = head;
            head = head->next;
            free(del->script);
            free(del->frame_ptr);
            free(del);
        }    
    }
    clearPageFrame();
}


void appendPCB(struct PCB *pcb){
   // for round robin, update pcb counter and append pcb to the end of queue
   struct PCB *current = head;

   //update pcb
   pcb->next = NULL;

   while(current->next!=NULL){
       current = current->next;
   }
   current->next = pcb;
}

void RoundRobin(){

    struct PCB *next = head;
    struct PCB *del = NULL;

    while(head != NULL){
        int isFinish = runProgram(head, 2);

        if(isFinish == 0){
            // program is finish
            del = head;
            head = head->next;
            free(del->script);
            free(del->frame_ptr);
            free(del);

        } else {
            // append to the end of queue
            if(head->next != NULL){
                // if head->next = NULL means that the cur program
                // is the only program left in the queue
                head = head->next;
                appendPCB(next);
            } 
        }
        next = head;
    }
    clearPageFrame();
 }   

void updateScore(){
    // decrease all the job's score by 1 except the job at the front of the queue
    struct PCB *current = head->next;
    while(current != NULL){
        if( current->score>0){
            current->score = (current->score)-1;
        }
        current=current->next;
    }
   
}

int updateQueue(){
    // find smallest one
    struct PCB *shortest = head;
    struct PCB *current = head->next;
    while(current != NULL){
        if(shortest->score > current->score){
            shortest = current;
        }
        current = current->next;
    }

    // update the queue
    // case 1. shortest one is the one in front or there could be only 1 job left
    if((shortest->PID) == (head->PID)){
        return 0;
    // case 2. shortest one is the last job
    } else if (shortest->next == NULL){
        // there are 3 programs in the queue
        if(head->next->PID != shortest->PID){
            shortest->next = head->next;
            shortest->next->next = head;
            head->next = NULL;
        } else {
        // there are 2 programs
            shortest->next = head;
            head->next = NULL;
        }   
    // case 3. shortest one is in the middle
    } else {
        shortest->next->next = head;
        head->next = NULL;
    }

    head = shortest;
    return 0;

}

 void SJFwithAging(){
     while(1){
         // execute 1 instruction of job at the front of the queue
         int isFinish = runProgram(head, 1);

         updateScore();

         if(isFinish == 0){
             struct PCB *del = head;
             head = head->next;
             clearShellMemory(del);
             free(del->script);
             free(del->frame_ptr);
             free(del);
         }

         if(head == NULL){
             break;
         }

         updateQueue();
     }
 }
