#include <stdio.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

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

void nameFunc(char *file, char* path){

    DIR *d = opendir(path);
    struct dirent *dir;

    while ((dir = readdir(d)) != NULL)
    {
      
      if(dir->d_type == DT_REG && file == dir->d_name ){
          printf("%s\n", dir->d_name);
        exit(0);
    }
    else
        if(dir->d_type == DT_DIR){
        
        char* d_path = strcat(path, dir->d_name);
    printf("%s\n",d_path);
            if(fork()==0){
                printf("%s", d_path);
                nameFunc(file,d_path);
                
            }
        }
    }
    closedir(d);
}



int c (int argc, char *argv[]){
    
 signal(SIGINT, sig_usr);
    

nameFunc (argv[1], "/home");   
    

}

int d(void)
{
  DIR           *d;
  struct dirent *dir;
  d = opendir(".");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
  {
     printf("%s\n", dir->d_name);
  }  
     
    }

    closedir(d);
  }

  return(0);
}



void show_dir_content(char * path, char * file)
{
    
  DIR * d = opendir(path); // open the path
  if(d==NULL) return; // if was not able return
  struct dirent * dir; // for the directory entries
  while ((dir = readdir(d)) != NULL) // if we were able to read somehting from the directory
    {
        if(dir -> d_type != DT_DIR && dir->d_name == file){
            printf("%s/%s\n", path,dir->d_name);
            //printf("%s\n", dir->d_name);
        }
        
        else 
            if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
      {
        if(fork()==0){
          printf("%s%s\n",GREEN, dir->d_name); // print its name in green
        char d_path[255]; // here I am using sprintf which is safer than strcat
        sprintf(d_path, "%s/%s", path, dir->d_name);
        show_dir_content(d_path,file); // recall with the new path
      }
      }
      //else
        //  if(dir-> d_type != DT_DIR) // if the type is not directory just print it with blue
        //printf("%s%s\n",BLUE, dir->d_name);
    }
    closedir(d); // finally close the directory
}

int main(int argc, char **argv)
{

  printf("%s\n", NORMAL_COLOR);

    show_dir_content(argv[1], argv[2]);

  printf("%s\n", NORMAL_COLOR);
  return(0);
}