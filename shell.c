#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "interpreter.h"
#include "shellmemory.h"
 
int framesize = FRAMESIZE;
int varmemsize = VARMEMSIZE;

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);
int separateInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();
	printf("Frame Store Size = %d; Variable Store Size = %d\n", framesize, varmemsize);

	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

	//init user input
	for (int i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	//init shell memory
	mem_init();

	//init page frame
	pageFrame_init();

	//init backingstore
	backingStore_init();

	while(1) {							
		printf("%c ",prompt);
		fgets(userInput, MAX_USER_INPUT-1, stdin);

        // if equal to \0 then it is end of file, redirect it to $	
		if(strcmp(userInput, "\0")==0){
			freopen("/dev/tty", "r", stdin);
		}

		separateInput(userInput);
		memset(userInput, 0, sizeof(userInput));

	}

	return 0;

}

int separateInput(char ui[]){
	char tmp[1000];
	int a,b;
	int errorCode = 0;

	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000){
		for(b=0; ui[a] != '\0' && ui[a] != ';'; a++, b++){
			tmp[b] = ui[a];
		}

		// out of loop means encounter ;
		// check if char before ; is a whitespace or not (invalid if whitespace)
		if(ui[a-1] == ' '){
			return badcommand();
		} 
		tmp[b] = '\0';
 
        errorCode = parseInput(tmp);
		 
		if(errorCode == -1) exit(99); // ignore all other errors
		else if(ui[a] == '\0') break;
		 
		a++;
	}
	return errorCode;
}

// Extract words from the input then call interpreter
int parseInput(char ui[]) {

	char tmp[200];
	char *words[100];							
	int a,b;							
	int w=0; // wordID
 
	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000) {

		for(b=0; ui[a]!='\0' && ui[a]!=' ' && a<1000; a++, b++)
			tmp[b] = ui[a];						// extract a word
	 
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		w++;

		if(ui[a] == '\0'){
			break;
		}
		a++;
	}

	return interpreter(words, w);
}
