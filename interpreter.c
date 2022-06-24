#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"
#include "kernel.h"

int MAX_ARGS_SIZE = 7;

int fileNumber = 0;

int help();
int quit();
int badcommand();
int badcommandTooManyTokens();
int badcommandFileDoesNotExist();
int badcommandInvalidFile();
int badcommandInvalidPolicy();
int notEnoughSpaceInShellMemory();
int set(char* var, char* value, int n);
int print(char* var);
int run(char* script);
int echo(char* val);
int my_ls();
int exec(char* command_args[], int args_size);
int resetmem();
int isAllAlphanumeric(char c[]);
int isAlphanumeric(char c);
char* concatenateVal(char* val[], int num);
int validPolicy(char* policy);
int validFile(char* command_args[], int args_size);


// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){  
	int i;

	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size < 3 || isAllAlphanumeric(command_args[1]) == 0) return badcommand();	
		else if (args_size > 7) return badcommandTooManyTokens();


		char* value = (char*)calloc(1,150);
		char spaceChar = ' ';

		for(int i = 2; i < args_size; i++){
			strncat(value, command_args[i], 30);
			if(i < args_size-1){
				strncat(value, &spaceChar, 1);
			}
		}
		
		int rt = set(command_args[1], value, 1);
		if(rt == -1){
			notEnoughSpaceInShellMemory();
		} 
		return 0;
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);

	} else if (strcmp(command_args[0], "echo")==0) {
		if (args_size != 2) return badcommand();
		return echo(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if(args_size != 1) return badcommand();
		return my_ls();

	} else if (strcmp(command_args[0], "exec")==0){
		// check if the 3 files are the same
		if(!validFile(command_args, args_size)){
			return badcommandInvalidFile();
		}else if(!validPolicy(command_args[args_size-1])){
			return badcommandInvalidPolicy();
		}else if(args_size == 3){
			return run(command_args[1]);
		} else if(args_size == 4 || args_size == 5){
			return exec(command_args, args_size);
		} else {
			return badcommand();
		}
	} else if (strcmp(command_args[0], "resetmem")==0){
		if(args_size != 1) return badcommand();
		return resetmem();
	} else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			          DESCRIPTION\n \
help			          Displays all the commands\n \
quit			          Exits / terminates the shell with “Bye!”\n \
set VAR STRING		          Assigns a value to shell memory\n \
print VAR		          Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		          Executes the file SCRIPT.TXT\n \
echo STRING                      Displays strings which are passed as arguments\n \
my_ls                            Lists all the files present in the current directory\n \
exec prog1 prog2 prog3 POLICY    Executes up to 3 concurrent programs, according to a given scheduling policy\n \
resetmem                         Deletes the content of the variable store\n";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");

	//remove backingstore
	backingStore_remove();

	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandInvalidFile(){
	printf("%s\n", "Bad command: Invalid File");
	return 1;
}

int badcommandInvalidPolicy(){
	printf("%s\n", "Bad command: Invalid policy");
	return 1;
}

int notEnoughSpaceInShellMemory(){
	printf("%s\n", "Bad command: Not enough space in shell memory");
	return 1;
}

int set(char* var, char* value, int n){

	char *link = "=";
	char buffer[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value);

	int rt = mem_set_value(var, value, n);
	return rt;

}

int print(char* var){
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

void createVarName(char *script, int n, char *varName){
	// create variable: fileNameLineNumber
	char num[5]; 
	sprintf(num, "%d", n);   // convert int that represent line number into string
	strcpy(varName, script);
	strcat(varName, "_");
	strcat(varName, num);
}

int loadSinglePage(char *script, int start){
	// store program line start into frame
	// store 1 page into frame..have start as start entry of frame
	// return frame number

	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		printf("cant find the file: %s\n", script);
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);

	int endOfFile = 0;
	int frameCounter = 1;
	int frameIndex = 0;
	int index;

	// loop until file is pointing at the "start" line
	for(int j = 1; j <= start; j++){
		if(feof(p)){
			return -1;  // end of file, nothing to load to pageframe
		}
		fgets(line,999,p);
	}

	int i = 0;
	while(i < 3){
		// code loading 
		char varName[100];
		createVarName(script, start, varName);

		// store variable and value which is the command on that line into shell memory
		if(feof(p)){ // last line of file
			endOfFile = 1;
		} else {
			line[strlen(line) - 1] = '\0'; // put null at the end of line (replace \n at the
			                               // end of each line except last line of file)
		}
		line[strlen(line)] = '\0';
		index = set(varName, line, 0);
		
		if(index == -1){	
			fclose(p);
			return -1;
		}
		start++;
		i++;
		if(endOfFile && i < 3){
			// if end of file but still have space left in the frame 
			// -> fill empty space
			for(; i < 3; i++){
				createVarName(script, start, varName);
				index = set(varName, "", 0);
				start++;
			}
		}
		fgets(line,999,p);
	}
	fclose(p);
	index = index/3;

	// return frame number
	return index;
}

int loadToFrameStore(char *script, int size, int isSort){

	int frameSize;

	// create pagetable
	if(size % 3 == 0){
		frameSize = size/3;
	} else {
		frameSize = (size/3) + 1;
	}
	int pageFrame[frameSize];

	// load the first two pages and assign -1 to the rest
	int start = 0;
	for(int i = 0; i < frameSize; i++){
		if(i < 2){
			int frameIndex = loadSinglePage(script, start);
		    pageFrame[i] = frameIndex;
		} else {
			pageFrame[i] = -1;
		}
		start += 3;
	}	
	createPCB(pageFrame, frameSize, size, isSort, script);
	return 0;
}

int loadToBackingStore(char *script, int isSort){
	char line;                
	FILE *p1 = fopen(script,"rt");  // the program is in a file

    // create a file(inside backing store) to write content of script into
	char num[5]; 
	sprintf(num, "%d", fileNumber);   // convert int that represent line number into string
	char varName[100];
	strcpy(varName, "backing store/file");
	strcat(varName, num);

	FILE *p2 = fopen(varName,"wt");

	if(p1 == NULL || p2 == NULL){
		return badcommandFileDoesNotExist();
	}

	// copy content of script into this file
	int size = 0; // keep track of how many line of codes are there in the file
	line = fgetc(p1);
	while(line != EOF){
		if(line == '\n'){
			size ++;
		}
		fputc(line, p2);
		line = fgetc(p1);
	}
	size ++;
	fileNumber++;

	fclose(p1);
	fclose(p2);

	loadToFrameStore(varName, size, isSort);

	return 0;
}


int run(char* script){

	loadToBackingStore(script, 0);  // load to backing store	
	FCFSandSJF();

	return 0;

}

int echo(char* val){

	if(val[0] == '$'){
		char *char_ptr = mem_get_value(val+1);
		if(strcmp(char_ptr, "Variable does not exist") == 0){
			printf("\n");
		} else {
			printf("%s\n", char_ptr);
		}
	} else {
		printf("%s\n", val);
	}
	return 0;
}

int my_ls(){
	char command[50];
	strcpy(command, "ls -l | awk '{if(NF == 9){print $NF}}'");
	system(command);
	return 0;
}

int exec(char* command_args[], int args_size){

	// create to PCB and load to ready queue
	int i = 1;
	int isSort = 0;
	if(strcmp(command_args[args_size-1], "SJF") == 0 || strcmp(command_args[args_size-1], "AGING") == 0){
		isSort = 1;
	}

	while(i < args_size - 1){
		// this loop start iterating from prog1 (command_args[1]) to last prog (command_args[args_size - 1])
		//int sucess = loadToBackingStore(command_args[i], isSort);
		loadToBackingStore(command_args[i], isSort);
		i++;
	}
	
	if(strcmp(command_args[args_size-1], "FCFS") == 0 || strcmp(command_args[args_size-1], "SJF") == 0){
		FCFSandSJF();
	} else if(strcmp(command_args[args_size-1], "RR") == 0){
		RoundRobin();
	} else if(strcmp(command_args[args_size-1], "AGING") == 0){
		SJFwithAging();
	}
	
	return 0;
	
}

int resetmem(){
	mem_reset_varStore();
	return 0;
}

int isAllAlphanumeric(char c[]){
	// check if given variable name is all alphanumeric
	int i = 0;
	while(c[i] != '\0'){
		if(isAlphanumeric(c[i]) == 0){
			return 0;
		}
		i++;
	}
	return 1;
}

int isAlphanumeric(char c){
	// 1 is true
	// 0 is false
	if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')){
		return 1;
	} else {
		return 0;
	} 
}

int validPolicy(char* policy){
	return strcmp(policy, "FCFS") == 0 || strcmp(policy, "SJF") == 0 || 
	       strcmp(policy, "RR") == 0 || strcmp(policy, "AGING") == 0;
}

int validFile(char* command_args[], int args_size){
	for(int i = 1; i < args_size-1; i++){
		
		// check if file exist
		FILE *p = fopen(command_args[i],"rt");  // the program is in a file
        if(p == NULL){
			return 0;
	    }
		fclose(p);
	}

	return 1;

}

