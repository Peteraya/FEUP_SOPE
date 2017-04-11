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


/*void commandExecution(char* cmd, char* path, char* dir_name, unsigned char dir_type){
    
    if(strcmp("-print",cmd)==0)
                printf("%s/%s\n", path, dir->d_name);
          
          else if(strcmp("-delete",cmd)==0){
           if(dir->d_type == DT_REG){
           char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              unlink(d_path);
          }
          else{
               char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              rmdir(d_path);
          }
          
          }
}*/

void nameFunc(char * executavel,char * path, char * mode, char * file, char * cmd)
{
     if(cmd==NULL)
     cmd="-print";  
     
    if(strcmp("d",file)==0 && strcmp("-type",mode)==0) {
           
                    if(strcmp("-print",cmd)==0)
                printf("%s\n", path);

                    
      }
         
         //printf("%s\n", path);
       
     
     /*else if(strcmp("-perm", mode) == 0) {
          struct stat fileStat;
          stat(path,&fileStat);
          //printf("%o \n",fileStat.st_mode);
          int statPerm = fileStat.st_mode&0777;
          char* ptr;
          if(statPerm == strtol(file, &ptr, 8))
               //printf("%s\n", path);
             //printf("%s/%s\n", path, dir->d_name);
          
      }*/
     
  DIR * d = opendir(path); 
  if(d==NULL) return; 
  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) 
    {
       
      if(strcmp(dir->d_name,file)==0 && strcmp("-name",mode)==0) {
            
          if(strcmp("-print",cmd)==0)
                printf("%s/%s\n", path, dir->d_name);
          
          else if(strcmp("-delete",cmd)==0){
           if(dir->d_type == DT_REG){
           char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              unlink(d_path);
          }
          else{
               char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              execlp("rmdir","rmdir",d_path,NULL);
          }
          
          }
            return;
      }
      
      
            if(strcmp("-type",mode)==0){
    
                
                if(dir -> d_type == DT_LNK && strcmp("l",file)==0) {
           
                    if(strcmp("-print",cmd)==0)
                printf("%s/%s\n", path, dir->d_name);
          
          else if(strcmp("-delete",cmd)==0){
           
           char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              unlink(d_path);
          
          }
                    
      }
        
        if(dir -> d_type == DT_REG && strcmp("f",file)==0) {
       
            if(strcmp("-print",cmd)==0)
                printf("%s/%s\n", path, dir->d_name);
          
          else if(strcmp("-delete",cmd)==0){
    
           char d_path[255]; 
        sprintf(d_path, "%s/%s", path, dir->d_name);
              unlink(d_path);
          }
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
        execlp(executavel,executavel,d_path, mode,file,cmd,NULL); 
      }
      }
      
    }
    
    if(strcmp("d",file)==0 && strcmp("-type",mode)==0) {
           
                    if(strcmp("-delete",cmd)==0)
                        execlp("rmdir","rmdir",path,NULL);

                    
      }
    closedir(d); // finally close the directory
}


void typeFunc(char * executavel, char * path, char * c)
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
}



int main(int argc, char **argv)
{
  // char* aux="/home";
     //signal(SIGINT, sig_usr);
  //printf("Arg 0 %s\n",argv[4]);
    
   /* if (strcmp(argv[2], "-perm") == 0) {
      char* ptr;
      octalValue = strtol(argv[3], &ptr, 8); 
    }*/
    
   nameFunc(argv[0],argv[1],argv[2], argv[3], argv[4]);
        
   // typeFunc(argv[0],argv[1],argv[2]);
    
     
  return(0);
}

