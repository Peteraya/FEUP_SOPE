#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define NORMAL_COLOR  "\x1B[0m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"

#define MAXCHAR 4096

char * init_path = "/home";

void sig_usr(int signo) { 

char res;

if (signo == SIGINT){ 
printf("Are you sure you want to terminate (Y/N)?\n");
scanf("%c",&res);

if(res=='Y' || res=='y'){
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}
	
}

}


void nameFunc(char * executavel,char * path, char * mode, char * file)
{
     
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) 
    {
        //printf("path: %s\n",path);
      if(strcmp(dir->d_name,file)==0 && strcmp("-name",mode)==0) {
        printf("%s/%s\n", path, dir->d_name);
        return;
      }
      else
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
      {
          pid_t pid=fork();
       if(pid>0)
           waitpid(pid, NULL, 0);
       
       else{
        char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
        execlp(executavel,executavel,d_path, mode,file,NULL); 
      }
      }
    }
    closedir(d); // finally close the directory
}


void typeFunc(char * executavel, char * path, char * c)
{
   

  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  
  while ((dir = readdir(d)) != NULL) 
    {
        
         if(dir -> d_type == DT_LNK && strcmp("l",c)==0) {
        printf("%s/%s\n", path, dir->d_name);
      }
        
        if(dir -> d_type == DT_REG && strcmp("f",c)==0) {
        printf("%s/%s\n", path, dir->d_name);
      }
     
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
      {
          if(strcmp(c,"d")==0)
            printf("%s/%s\n", path,dir->d_name);
          
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
}


int main(int argc, char **argv)
{
  // char* aux="/home";
     //signal(SIGINT, sig_usr);
  // printf("Arg 0 %s\n",argv[1]);
    
    // nameFunc(argv[1],argv[2],argv[0]);
    
//     nameFunc(argv[0],argv[1],argv[2], argv[3]);
     
    typeFunc(argv[0],argv[1],argv[2]);
     
  return(0);
}

