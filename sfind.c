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
static pid_t pid_handler; //variavel global
struct sigaction oldsigaction; //declarar antes de subscribe_SIGINT


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


void sig_handler(int sig){
	if(sig == SIGINT){

		char res;

	if(getpid()==pid_handler) {
		write(STDOUT_FILENO, "\nAre you sure you want to terminate (Y/N)?\n", 44); 
	}
		
	kill(0, SIGTSTP); //envia sinal Stop ao processo pai

		do{
			read(STDIN_FILENO, &res, 1);
			if (res == 'Y' || res == 'y')
			{
				kill(0,SIGTERM); //envia sinal de Terminacao ao proceso pai
			}else
				if (res == 'N' || res == 'n')
				{
					kill(0,SIGCONT); //envia sinal ao processo pai para continuar
					break;
				}
		}while(res != 'Y' && res != 'y' && res != 'N' && res != 'n');
	}

}

void subscribe_SIGINT(){

    sigaction(SIGTSTP, 0,&oldsigaction);
	struct sigaction newsigaction;

	newsigaction = oldsigaction;
	newsigaction.sa_handler = &sig_handler;

	sigaction(SIGTSTP, &newsigaction,0);
	signal(SIGINT, sig_handler);

	
}





void commandExecutionFiles(char* cmd, char* path, char* dir_name, unsigned char dir_type,char* execCommand){
    
    char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir_name);
    
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

void nameFunc(char * executavel,char * path, char * mode, char * file, char * cmd,char * execCommand)
{
     if(cmd==NULL)
     cmd="-print";  
     
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
                            printf("%s\n", path);
                    
}
            
            
     
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) 
    {
       
        if(strcmp("-perm", mode) == 0 && dir -> d_type == DT_REG ) {
                    struct stat fileStat;
                    char dir_path[255]; 
                    sprintf(dir_path, "%s/%s", path, dir->d_name);
                    
                    stat(dir_path,&fileStat);
                    char * ptr;
                    int statPerm = fileStat.st_mode&0777;
            
                    //printf("Perm: %o\n", statPerm);
                    
                    if(statPerm == strtol(file, &ptr, 8) && !(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0 || (dir->d_name[0] == '.')))
                            printf("%s\n", dir_path);
                    
}
      
        
        
      if(strcmp(dir->d_name,file)==0 && strcmp("-name",mode)==0) {
            
          commandExecutionFiles(cmd, path, dir->d_name,dir->d_type,execCommand);
            return;
      }
      
      
            if(strcmp("-type",mode)==0){
    
                
                if(dir -> d_type == DT_LNK && strcmp("l",file)==0) 
                    commandExecutionFiles(cmd, path, dir->d_name,dir->d_type,execCommand);
                    
    
        if(dir -> d_type == DT_REG && strcmp("f",file)==0){
            
            
            commandExecutionFiles(cmd, path, dir->d_name,dir->d_type,execCommand);
           
            }
            
            
            }
    
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
      {
          pid_t pid=fork();
       if(pid>0)
           waitpid(pid, NULL, 0);

       else{
        char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
        execlp(executavel,executavel,d_path, mode,file,cmd,execCommand,NULL); 
      }
      }
      
    }
    
    if(strcmp("d",file)==0 && strcmp("-type",mode)==0) {
           
                    if(strcmp("-delete",cmd)==0)
                        execlp("rmdir","rmdir",path,NULL);
                    
    
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
   
    subscribe_SIGINT();
	pid_handler=getpid();
    
    nameFunc(argv[0],argv[1],argv[2], argv[3], argv[4],argv[5]);
    
  
  exit(0);
}

