#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

bool use_inode = false;
bool use_name = false;
int type_size = -2;
bool use_links = false;
bool use_exec = false;

ino_t file_ino;
char * file_name;
off_t file_size;
nlink_t file_nlink;
char * exec_path;

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

void checkNextArg(int cur,int num_args,char ** argv){
	if (cur + 1 >= num_args){
		printf("Argument %s has no param\n",argv[cur]);
		exit(0);
	}
			
}

int parseToInt(char * str){
	int ans = 0;
	bool neg = str[0] == '-';
	if (neg){
		 str++;
	}
	while (	isdigit(*str)){
		ans = ans * 10 + *str - '0';
		str++;
	}	
	if (neg){
		ans = -ans;
	}
	if (*str) {
		printf("It's not a number %d\n",ans);
		exit(0);
	}
	return ans;
}

int compareSize(off_t x,off_t y){
	if (x == y) return 0;
	else return x < y ? 1 : -1;
}

bool filter(char * path,char * dname){
	struct stat sb;	
	if (stat(path,&sb) == -1){
		perror("stat");
		return 0;
	}
	if (use_inode && sb.st_ino != file_ino){
		return false;
	}
	if (use_name && strcmp(file_name,dname)){
		return false;	
	}
	if (type_size != -2 && compareSize(file_size,sb.st_size) != type_size){
		return false;
	}
	if (use_links && sb.st_nlink != file_nlink){
		return false;
	}
	return true;
}


void find(char * path){
	DIR *d = opendir(path);
	if (d == NULL) return;
	struct dirent *dir;
	while ( (dir = readdir(d)) != NULL){
		char tmp[2048];
		snprintf(tmp,sizeof(tmp),"%s/%s",path,dir->d_name);
		if (dir->d_type != DT_DIR){	
			if (filter(tmp,dir->d_name)){
				if (use_exec){
					char * args[3] = {exec_path,tmp,NULL};
					launch(args);
				}
				else{
					printf("%s\n",tmp);
				}
					
			}
		}
		else if (dir->d_type == DT_DIR && strcmp(dir->d_name,".") != 0
			 && strcmp(dir->d_name,"..") != 0){	
			find(tmp);
		}	
	}	
	closedir(d);
}

int main(int argc,char * argv[]){
	if (argc <= 1) {
		printf("DE FUQ ?\n");
		exit(0);
	}
	for (int i = 2;i < argc;i += 2){
		checkNextArg(i,argc,argv);
		if (!strcmp(argv[i],"-inum") ){
			use_inode = true;
			file_ino = parseToInt(argv[i + 1]);
		}
		if (!strcmp(argv[i],"-name") ){
			use_name = true;
			file_name = argv[i + 1];
		}
		if (!strcmp(argv[i],"-size") ){
			if (strlen(argv[i + 1]) < 2){
				printf("Such size is not exist	");
				exit(0);
			}
			if (*argv[i + 1] == '-'){
				type_size = -1;	
			}
			else if (*argv[i + 1] == '='){
				type_size = 0;
			}
			else if (*argv[i + 1] == '+'){
				type_size = 1;
			}
			else {
				printf("Wrong type of size");
				exit(0);
			}
			file_size = parseToInt(argv[i + 1] + 1);
		}
		if (!strcmp(argv[i],"-nlinks") ){
			use_links = true;
			file_nlink = parseToInt(argv[i + 1]);
		}
		if (!strcmp(argv[i],"-exec") ){
			use_exec = true;
			exec_path = argv[i + 1];	
		}				
	}	
	find(argv[1]);
}
