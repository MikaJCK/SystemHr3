#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#define MAX_COMMANDS 50

int GetArrayLenght(char * array[]){
	int i=0;
	while(array[i]!=NULL){
		i++;
	}
	return i;	
}

void printArray(char *array[]){
	int i=0;
	while(array[i]!=NULL){
		printf("%s\n", array[i]);
		i++;
	}

}

void quickDebug(int delt){
	int proxy;
	printf("%d\n",delt);
	scanf("%d", &proxy);
}

void clearArray(char *array[], int point){
	int i=point;
	while(array[i]!=NULL){
		//printf("i:%d:%s\n",i,array[i]);
		free(array[i]);
		i++;
	}
}

//used to parse the command into a form that exec can run
//if the main command is in form of \location\command then 0 is returned along with the original
//command and path
int ParseMainCommand(char *mainCommand, char * paths[], char* command,int version){
	int i=0;
	if(version==0){
		strcpy(command,mainCommand);
		return 1;
	}
	while(*(mainCommand+i)!='\0'){
		if(*(mainCommand+i)=='/'){
			strtok(command,mainCommand);
			printf("return zero\n");
			return 0;
		}
		i++;
	}
	
	i=0;
	sprintf(command, "%s%s",paths[version-1],mainCommand);
	return 1;
}

// parse a full line command into singewords on an array. So they are divided based on a whitespace
void parseCommand(char cmd[255], char *array[]){
	char *proxy=NULL;
	if((proxy=malloc(sizeof(char[255])))==NULL){
		printf("Error allocating memory: %s.\n", strerror(errno));
		exit(1);
	}
	int counter=0,i=0, offset=0;
	while(cmd[i]!='\n'){
		
		if(cmd[i]==' ' && (cmd[i+1]==' '|| i==0)){//check for multiple whitespace cmd[i+1]=='\n'
			i++;
			continue;
		}
		if(cmd[i]==' ' ||cmd[i+1]=='\n'){
			if(cmd[i+1]=='\n'){
				*(proxy+(i-offset))=cmd[i];	
				if(cmd[i]==' '){
					proxy[i-offset]='\0';
				}else{
					proxy[i+1-offset]='\0';
				}
			}else{
				proxy[i-offset]='\0';
			}
			
			array[counter]=proxy;
			offset=i+1; //offset is used to make sure that the character in the else section
						//is put onto the correct spot
			counter++;
			if((proxy=malloc(sizeof(char[255])))==NULL){
				printf("Error allocating memory: %s.\n", strerror(errno));
				exit(1);
			}
			i++;
		}else{
			*(proxy+(i-offset))=cmd[i];
			i++;
		}
		
		
		
	}
	
	array[counter]=NULL;
	
}


//
int SizeofArray(char *array[0]){
	int i=0;
	while(array[i]!=NULL){
		i++;
	}
	return i;
}

//used to exit in case of long period of doint nothing
void Alarmed(int sign){
	signal(SIGALRM,SIG_IGN);
	puts("AFK for too long\n");
	exit(1);
}

//used to check for | and & flags so that the functions can be run either by paraller or in line
int ParallerFlagCheck(char *cmdArray[]){
	int i=0;
	while(cmdArray[i]!=NULL){
		if(strcmp(cmdArray[i],"&")==0){
			return 1;
		}else if(strcmp(cmdArray[i],"|")==0){
			return 1;
		}
		i++;
	}
	
	return 0;
}

void Leave(){
	exit(1);
}

//clears the stdin buffer for a second(shoud be enough for most buffers)
void clearer(int fg){
	int rc=fork();
	int c;
	signal(SIGALRM, Leave);
	if(rc<0){
		fprintf(stderr, "fork() failed\n");
		exit(1);
	}else if(rc==0){
		alarm(1);
		if(fg==0){
		int timer=time(0)+1;
		while(timer>time(0)){
					
			c=getchar();
			if(c==0){
				break;
			}
		}
		exit(1);
	}			
				
	}else{
		wait(NULL);
		
	}

	
}

//used to excecute given commandArray. bg int is a lfag that tells that this will be done it\s own fork otherwise the program waits for the child fork to finish
int Excecute(int bg, int fg, char *commandArray[], char *paths[]){
	printf("excecuting %s\nwith parameters: \n",commandArray[0]);
	int i=1;
	while(commandArray[i]!=NULL){
		printf("%d : %2s \n", i, commandArray[i]);
		i++;
	}
	int currentpid;
	int rc=fork();
		if(rc<0){
			fprintf(stderr, "fork() failed\n");
			exit(1);
		}else if(rc==0){
			int j=0;
			int stop=0, fail=0;
			char *MainCommand=NULL;
			if((MainCommand=malloc(sizeof(char)*255))==NULL){
				printf("Error allocating memory: %s.\n", strerror(errno));
				exit(1);
			}
			while(paths[j-fail]!=NULL){
				if(ParseMainCommand(commandArray[0], paths, MainCommand, fail)==0){
					stop=1;
				}
				if(execv(MainCommand, commandArray)==-1 ){
					
				}
				
				if(stop && fail){
					printf("stop and failed\n");
					fprintf(stderr, "error executing %s: %s\n", commandArray[0], strerror(errno));
					exit(1);
				}else if(stop){
					printf("stop \n");
					exit(0);
				}
				fail=1;	
				j++;
				
			}
			printf("error executing %s\n", commandArray[0])
			
		}else{
			
			if(bg==1 && fg==1){
				currentpid=rc;
				
			}
			
			else{
				wait(NULL);
				
			}
			if(kill(currentpid,0)==-1){
				return 0;
			}
		}
		return 1;
}


//ran if there is paraller launch
void RunParaller(char *array[], char *paths[]){
	int i=0,j=0;
	char *proxyCMD[MAX_COMMANDS];
	
	while(array[i]!=NULL){
		
		
		if(strcmp(array[i],"&")==0){
			proxyCMD[j]=NULL;
			
			Excecute(1,1, proxyCMD,paths);
			j=0;
		}else if(strcmp(array[i],"|")==0){//if there is a |, the function is ran normally
			proxyCMD[j]=NULL;
			Excecute(0,1, proxyCMD, paths);
			j=0;
		}else{
			
			proxyCMD[j]=array[i];
			printf("%s\n", proxyCMD[j]);	
			
			j++;
		}
		i++;
		if(array[i]==NULL){
			if(j==0){
				return;
			}
			proxyCMD[j]=NULL;
			Excecute(0,1, proxyCMD,paths);
		}
		
	}
	//Excecute(1,1, proxyCMD);
}

//adds the paths to the paths array
void MakePath(char * cmdArray[], char* paths[]){
	int i=1;
	char *test=NULL;
 	if((test=malloc(sizeof(char[255])))==NULL){
		printf("Error allocating memory: %s.\n", strerror(errno));
		exit(1);
	}
	
	while(cmdArray[i]!=NULL){
		strcpy(test,cmdArray[i]);
		
		paths[i]=test;
		
		if((test=malloc(sizeof(char[255])))==NULL){
			printf("Error allocating memory: %s.\n", strerror(errno));
			exit(1);	
		}
		i++;
	}
	if(i==1){
		printf("Not enough arguments. Path not set\n");
		printf("current paths: ");
		printArray(paths);
		printf("\n");
	}
	paths[i]=NULL;
}

void ChangeDirectory(char * array[]){
	if(GetArrayLenght(array)<2){
		printf("no directory specified\n");
		
		return;
	}
	if(chdir(array[1])==-1){
		printf("Error in changing directory: %s\n", strerror(errno));
	}
}

char** ParseAndStartExce(char *cmd, char *paths[]){
	char *commandArray[MAX_COMMANDS];
	int bg=0, fg;
	parseCommand(cmd, commandArray);
	if((fg=ParallerFlagCheck(commandArray)) && (bg!=1)){
		bg=1;
	}
	
	if(strcmp(commandArray[0],"exit")==0){
		clearArray(paths, 1);
		Leave();
	}
	if(strcmp(commandArray[0],"path")==0){
		MakePath(commandArray, paths);
	}else if(strcmp(commandArray[0],"cd")==0){
		ChangeDirectory(commandArray);
	}else if(bg){
		RunParaller(commandArray, paths);
	}else{
		Excecute(bg,fg,commandArray, paths);
	}
	if(fg==0){
		clearer(fg);
		signal(SIGALRM, Alarmed);
	}
	clearArray(commandArray,0);
	return paths;
}

void BatchMode(char* filename, char *paths[]){
	FILE *fl;
	if((fl=fopen(filename,"r"))==NULL){
		fprintf(stderr, "Error\n" );
		exit(1);
	}
	char line[255];
	while(fgets(line, 255, fl)!=NULL){
		ParseAndStartExce(line, paths);
	}
}

int main(int argc, char * argv[]){
	
	char *paths[MAX_COMMANDS];
	paths[0]="/bin/";
	paths[1]=NULL;
	char **ptr=NULL;
	if((ptr=malloc(sizeof(char**)))==NULL){
		printf("Error allocating memory: %s.\n", strerror(errno));
		exit(1);
	}
	
	ptr=paths;
	if(argc==2){
		BatchMode(argv[1], paths);
	}
	char cmd[255];
	
	signal(SIGALRM, Alarmed);
	while(1){
		alarm(30);
		
		setbuf(stdin,NULL);
		printf("wish>");
		fgets(cmd, sizeof(cmd), stdin);
		
			
			ptr=ParseAndStartExce(cmd, ptr);	
		
		
			
	}
	clearArray(paths,1);

}

