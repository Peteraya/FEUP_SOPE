#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>


struct mensagem_pedido{
  int p;
  char g;
  int t;
    
    
};



int main(int argc, char **argv)
{
     if (mkfifo("/tmp/rejeitados",0660)<0) 
        if (errno==EEXIST) printf("FIFO '/tmp/rejeitados' already exists\n"); 
        else
            printf("Can't create FIFO\n"); 
        

    
  
  
}

