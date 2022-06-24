#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"shell.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<dirent.h>

struct memory_struct{
	char *var;
	char *value;
};

void printPageFault(int frame);
struct memory_struct shellmemory[FRAMESIZE+VARMEMSIZE];
int pageFrame[FRAMESIZE/3]; 

// Shell memory functions
void mem_init(){
	int i;
	for (i=0; i<FRAMESIZE+VARMEMSIZE; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

void backingStore_remove(){
	char command[50];

	strcpy(command, "rm -rf \"backing store\"");
	system(command);
}

int backingStore_init(){
	backingStore_remove();
	// create directory
	int fail = mkdir("backing store", 0777);

	// check if success
	if(fail){
		printf("cannot create directory\n");
		return 1;
	}
	return 0;

}

void pageFrame_init(){
	for(int i = 0; i < FRAMESIZE/3; i++){
		pageFrame[i] = i;
	}
}

void mem_reset_codeloading(){
	int i;
	for(i=0; i<FRAMESIZE; i++){
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

void mem_reset_varStore(){
	int i;
	for(i = (FRAMESIZE+VARMEMSIZE)-1; i >= FRAMESIZE; i--){
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

void mem_reset_index(int index){
	shellmemory[index].var = "none";
	shellmemory[index].value = "none";
}

// Set key value pair
int mem_set_value(char *var_in, char *value_in, int n) {

	int i = -1;

	for (i=0; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return i;
		} 
	}

	// find free space from the beginning 
	if(n == 0){
		//Value does not exist, need to find a free spot (for code loading)
		for (i=0; i<FRAMESIZE; i++){
			if (strcmp(shellmemory[i].var, "none") == 0){
				shellmemory[i].var = strdup(var_in);
				shellmemory[i].value = strdup(value_in);
				return i;
			} 
		}
	// find free spot from last index (for variables)
	} else {
		for(i = (FRAMESIZE+VARMEMSIZE)-1; i >= FRAMESIZE; i--){
			if (strcmp(shellmemory[i].var, "none") == 0){
				shellmemory[i].var = strdup(var_in);
				shellmemory[i].value = strdup(value_in);
				return i;
			} 
		}
	}

	return -1; // if cannot find free spot, it will return -1

}

//get value based on input key
char *mem_get_value(char *var_in) {
	int i;

	for (i=0; i<FRAMESIZE+VARMEMSIZE; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){

			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";
}

void printframe(){
	for(int i = 0; i < FRAMESIZE/3; i++){
		printf("%d ", pageFrame[i]);
	}
	printf("\n");
}

void updateRecentlyUsed(int frame){
	// bring the entry at the specify index to 
	// the front of array

	// find the index of frame
	int i = 0;
	for(; i < FRAMESIZE/3; i++){
		if(pageFrame[i] == frame){
			break;
		}
	}

	for(int j = i - 1; j >= 0; j--){
		pageFrame[j+1] = pageFrame[j];
	}

	pageFrame[0] = frame;
}

void runIndex(int frameIndex, int lineIndex){
	// i is the index inside shell memory

	// update the number of page frame access
	updateRecentlyUsed(frameIndex);

	int index = frameIndex * 3;
	index += lineIndex;

	// run as command the value at index i 
	separateInput(shellmemory[index].value);
}

char* evictLeastRecentlyUsed(int* evictedPageFrame){
	// evict least recently used page frame
	int leastRecentlyUsed = pageFrame[(FRAMESIZE/3)-1];
	*evictedPageFrame = leastRecentlyUsed;

	// find the file name that we evict
	char *fileName = malloc(sizeof(char)*10);
	fileName = strtok(shellmemory[leastRecentlyUsed*3].var, "_");

	return fileName;
}

void printPageFault(int frame){
	printf("Page fault! Victim page contents:\n");
	int index = frame*3;
	for(int i = 0; i < 3; i++){
		printf("%s\n", shellmemory[index].value);
		index++;
	}
	printf("End of victim page contents.\n");
}


