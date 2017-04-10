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


void namefunc(char * path,char * file, char * executavel)
{
     
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) 
    {//printf("Path: %s\n", path);
      if(strcmp(dir->d_name,file)==0) {
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
        execlp(executavel,executavel,d_path, file,NULL); 
      }
      }
    }
    closedir(d); // finally close the directory
}

int main(int argc, char **argv)
{
     signal(SIGINT, sig_usr);
 
     namefunc(argv[1],argv[2],argv[0]);
     
     
  return(0);
}