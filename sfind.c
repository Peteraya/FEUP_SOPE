#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define NORMAL_COLOR  "\x1B[0m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"

#define MAXCHAR 4096

unsigned long octalValue;


/*void sigint_handler(int signo) { 

char res;

if (signo == SIGINT){ 


printf("Are you sure you want to terminate (Y/N)?\n");
scanf("%c",&res);

if(res=='Y' || res=='y'){
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	
}

}*/


static pid_t pid; //variavel global
struct sigaction oldsigaction; //declarar antes de subscribe_SIGINT

static pid_t pids[4000];




void sig_handler(int sig){
	if(sig == SIGINT){

		char res;
                


                

	if(getpid()==pids[0]) {
            //printf("\nPID: %d\n",getpid());
                write(STDOUT_FILENO, "\nAre you sure you want to terminate (Y/N)? ", 44); 
		
	}
	

		do{
			read(STDIN_FILENO, &res, 1);
			if (res == 'Y' || res == 'y')
			{
				kill(0,SIGTERM); //envia sinal de Terminacao ao proceso pai
			}
			else if(res == 'n' || res == 'n')
                               for(int i=0; i< (sizeof(pids)/sizeof(pids[0]));i++)
                                            kill(pids[i],SIGCONT);
		}while(res != 'Y' && res != 'y' && res != 'N' && res != 'n');
	}

}

//chamar esta função logo no incicio do main
void subscribe_SIGINT(){

    sigaction(SIGTSTP, 0,&oldsigaction);
	struct sigaction newsigaction;

	newsigaction = oldsigaction;
	newsigaction.sa_handler = &sig_handler;

	sigaction(SIGTSTP, &newsigaction,0);
	signal(SIGINT, sig_handler);

	
}






void commandExecutionFiles(char* cmd, char* d_path, unsigned char dir_type,char* execCommand){
    
    
            if(strcmp("-print",cmd)==0)
                printf("%s\n", d_path);
            
             if(strcmp("-exec",cmd)==0){
                if(fork()==0)
                    execlp(execCommand,execCommand, d_path,NULL);
                else
                    wait(NULL);
             }
          
          else if(strcmp("-delete",cmd)==0){
           
              
              if(dir_type == DT_REG){
           
              unlink(d_path);
          }
          else{
              
              execlp("rmdir","rmdir",d_path,NULL);
          }
          
          }
          
}

void nameFunc(char * executavel,char * path, char * mode, char * file, char * cmd,char * execCommand, char* vecPos)
{
    
    
    int vecPosInt;
    
    if(strcmp(vecPos,"XDG_VTNR=7")==0){
     vecPosInt=0;
        
    }
    else{
        vecPosInt = atoi(vecPos);
        vecPosInt++;    
    }
    
    //printf("%d\n",vecPosInt);
    
     if(cmd==NULL)
     cmd="-print"; 
     
     pids[vecPosInt]=getpid();
     
     
    if(strcmp("d",file)==0 && strcmp("-type",mode)==0) {
           
                    if(strcmp("-print",cmd)==0)
                printf("%s\n", path);

                if(strcmp("-exec",cmd)==0){
                if(fork()==0)
                    execlp(execCommand,execCommand, path,NULL);
                else
                    wait(NULL);
             }
                    
      }
            
            
             if(strcmp("-perm", mode) == 0) {
                    struct stat fileStat;
                   
                    
                    stat(path,&fileStat);
                    char * ptr;
                    int statPerm = fileStat.st_mode&0777;
            
                    //printf("Perm: %o\n", statPerm);
                    
                    if(statPerm == strtol(file, &ptr, 8) && !(strcmp(path,".") == 0 || strcmp(path,"..") == 0 || (path[0] == '.')))
                    {
                        
                    if(strcmp("-print",cmd)==0)
                printf("%s\n", path);

                if(strcmp("-exec",cmd)==0){
                if(fork()==0)
                    execlp(execCommand,execCommand, path,NULL);
                else
                    wait(NULL);
             }
                    
      }
                        
                        
                    }
                                
     
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) 
    {
       
        char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
        
        
        if(strcmp("-perm", mode) == 0 && dir -> d_type == DT_REG ) {
                    struct stat fileStat;
                    
                    
                    stat(d_path,&fileStat);
                    char * ptr;
                    int statPerm = fileStat.st_mode&0777;
            
                    
                    if(statPerm == strtol(file, &ptr, 8) && !(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0 || (dir->d_name[0] == '.')))
                            commandExecutionFiles(cmd, d_path,dir->d_type,execCommand);
                    
}
      
        
        
      if(strcmp(dir->d_name,file)==0 && strcmp("-name",mode)==0) {
            
          commandExecutionFiles(cmd, d_path,dir->d_type,execCommand);
            return;
      }
      
      
            if(strcmp("-type",mode)==0){
    
                
                if(dir -> d_type == DT_LNK && strcmp("l",file)==0) 
                    commandExecutionFiles(cmd, d_path,dir->d_type,execCommand);
                    
    
        if(dir -> d_type == DT_REG && strcmp("f",file)==0)
            commandExecutionFiles(cmd, d_path,dir->d_type,execCommand);
           
            }
    
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
      {
          pid_t pid=fork();
       if(pid>0)
           waitpid(pid, NULL, 0);

       else{
        
        sprintf(vecPos,"%d",vecPosInt);
        execlp(executavel,executavel,d_path, mode,file,cmd,execCommand,vecPos, NULL); 
      }
      }
      
    }
    
    if(strcmp("d",file)==0 && strcmp("-type",mode)==0) {
           
                    if(strcmp("-delete",cmd)==0)
                        execlp("rmdir","rmdir",path,NULL);
                    
    
      }
      
        if(strcmp("-perm", mode) == 0) {
                    struct stat fileStat;
                   
                    
                    stat(path,&fileStat);
                    char * ptr;
                    int statPerm = fileStat.st_mode&0777;
            
                    //printf("Perm: %o\n", statPerm);
                    
                    if(statPerm == strtol(file, &ptr, 8) && !(strcmp(path,".") == 0 || strcmp(path,"..") == 0 || (path[0] == '.')))
                    {
                        
                    if(strcmp("-delete",cmd)==0)
                        execlp("rmdir","rmdir",path,NULL);
                    
      }
                        
                        
                    }
      
      
      
    closedir(d); // finally close the directory
}


/*void typeFunc(char * executavel, char * path, char * c)
{
   


 if(strcmp(c,"d")==0)
           printf("%s\n", path);
 
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  
  while ((dir = readdir(d)) != NULL) 
    {

         if(dir -> d_type == DT_LNK && strcmp("l",c)==0) {
        printf("%s/%s\n", path, dir->d_name);
      }
        
        else if(dir -> d_type == DT_REG && strcmp("f",c)==0) {
        printf("%s/%s\n", path, dir->d_name);
      }
     
      else if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
      {
        
          pid_t pid=fork();
       if(pid>0)
           waitpid(pid, NULL, 0);
       
       else{
        char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
        execlp(executavel,executavel,d_path,c, NULL); 
      }
      }
    }
    
    
    closedir(d); // finally close the directory
}*/



int main(int argc, char **argv)
{
  
   // signal(SIGINT, sigint_handler);

    
   /* if (strcmp(argv[2], "-perm") == 0) {
      char* ptr;
      octalValue = strtol(argv[3], &ptr, 8); 
    }*/
   
   subscribe_SIGINT();
    pid=getpid();
    
    
   // printf("Arg 6: %s\n", argv[6]);
   nameFunc(argv[0],argv[1],argv[2], argv[3], argv[4],argv[5],argv[6]);
   
   /* int i =0;
  while(1){
      
   printf("%d\n",i++); 
   sleep(1);
  }*/
  exit(0);
}

