#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

char * getLine(){
	char * line = NULL;
	size_t len = 0;
	ssize_t nread = getline(&line, &len, stdin);
	if (nread == -1){
		printf("\n");
		exit(0);
	}
	return line;
}

void checkAlloc(char ** args){
	if (!args){
		fprintf(stderr,"Allocation error\n");
		exit(1);
	}
}

char ** getArguments(char * line){
	int buf_size = 32;
	int pos = 0;
	char ** args = (char**)malloc(buf_size * sizeof(char*) );
	checkAlloc(args);
	if (line == NULL || line[0] == '\n'){
		args[0] = "exit";
		return args;
	}
	char * arg = strtok(line," \t\r\n\a");
	while (arg != NULL){
		args[pos] = arg;
		pos++;
		if (pos >= buf_size){
			buf_size += 32;
			args = (char**)realloc(args,buf_size * sizeof(char*) );
			checkAlloc(args);
		}
		arg = strtok(NULL," \t\r\n\a");
	}
	args[pos] = NULL;
	return args;	
}

int launch(char ** args){
	if (!strcmp(args[0],"exit") ) {
		return 0;
	}
	int status;
	pid_t pid = fork();
	if (pid == 0){
		char * envp[] = {NULL};
		if (execve(args[0],args,envp) == -1){
			perror("Launch error");
		}	
		exit(2);
	}
	else if (pid < 0) {
		perror("Fork error");
	}
	else {
		do {
			pid_t wpid = waitpid(pid,&status,WUNTRACED);
			if (wpid == 0){
				perror("Error");		
			}
			else if (wpid == -1){
				perror("procces waiting error");	
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int main( int argc,char* argv[] ){
	if (argc > 1){
		printf("No arguments expected\n");
		return 0;	
	}
	char * inputline = NULL;
	char ** arguments = NULL;
	int status;
	do {
		printf("->");
		inputline = getLine();
		arguments = getArguments(inputline);
		status = launch(arguments);
		free(inputline);
		free(arguments);
	} while(status)	;
	return 0;
}
