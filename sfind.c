#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>



#define MAXL 4000

static pid_t pids[MAXL];




void sig_handler(int sig){
	if(sig == SIGINT){

            
            	kill(0, SIGTSTP); 

		char res;
		if(getpid()==pids[0]) {
                write(STDOUT_FILENO, "\nAre you sure you want to terminate (Y/N)? ", 44); 
               
	}
	
	
                
                
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

struct sigaction oldsigaction; 


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

void nameFunc(char * executavel,char * path, char * mode, char * file, char * cmd,char * execCommand,char* vecPos)
{
    
    
    int vecPosInt;
    
    if(strcmp(vecPos,"XDG_VTNR=7")==0){
     vecPosInt=0;
        
    }
    else{
        vecPosInt = atoi(vecPos);
    }
    
    pids[vecPosInt++]=getpid();
    
    //printf("EXEC %s\n",execCommand);
    
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
            
                   
                    
                    if(statPerm == strtol(file, &ptr, 8))
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
        execlp(executavel,executavel,d_path, mode,file,cmd,execCommand,vecPos,NULL); 
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
            
                    
                    
                    if(statPerm == strtol(file, &ptr, 8))
                    {
                        
                    if(strcmp("-delete",cmd)==0)
                        execlp("rmdir","rmdir",path,NULL);
                    
      }
                        
                        
                    }
      
      
      
    closedir(d); 
}




int main(int argc, char **argv)
{
  
   
   
   subscribe_SIGINT();
    
   nameFunc(argv[0],argv[1],argv[2], argv[3], argv[4],argv[5],argv[6]);
    
  
  exit(0);
}

