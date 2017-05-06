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
#include <errno.h>
#include <sys/file.h> 

#define MAXL 4000

//Generos diferentes sem mutex?

sem_t sauna_full;

int fd_registos;

int nr_pedidos_global_F=0;
int nr_rejeitados_global_F=0;
int nr_servidos_global_F=0;

int nr_pedidos_global_M=0;
int nr_rejeitados_global_M=0;
int nr_servidos_global_M=0;


struct mensagem_pedido{
    int pedido;
    char gen;
    int tempo;
    int rejei;
};


void* entrar_sauna (void* arg){
    

	char msg[MAXL];
    struct mensagem_pedido *ms_ped = (struct mensagem_pedido*)arg;

     
    if(sem_wait(&sauna_full)){
		perror("sem_wait_error");
		exit(1); 
	}
  

    sleep(ms_ped->tempo/1000);
    
    sprintf(msg, "%d - %d - %lu - %d: %c - %d - SERVIDO \n",8, getpid(), pthread_self(), ms_ped->pedido,ms_ped->gen,ms_ped->tempo);
    
    
    write(fd_registos, msg, strlen(msg));

    if(ms_ped->gen=='F'){
        nr_servidos_global_F++;    
    }
    else{
        nr_servidos_global_M++;
    }
    
    if(sem_post(&sauna_full)){
		perror("sem_post_error");
		exit(1); 
	}
	
    return NULL;
}


int main(int argc, char **argv)
{
    
    if(sem_init(&sauna_full, 0, atoi(argv[1]))){
		perror("sem_init_error");
		exit(1); 
	}
    
    
    if (mkfifo("/tmp/rejeitados",0660)<0){ 
        if (errno==EEXIST) 
            printf("FIFO '/tmp/rejeitados' already exists\n"); 
        else
            printf("Can't create FIFO\n"); 
    }
        

	int fd_entrada, i=0;
	pthread_t tid[MAXL];
  
	char regis[MAXL], msg[MAXL];
    
	sprintf(regis,"/tmp/bal.%d",getpid());

	fd_entrada = open("/tmp/entrada",O_RDONLY);
    fd_registos = open(regis, O_WRONLY | O_CREAT | O_EXCL, 0644);
    
    struct mensagem_pedido *ms_ped;
    
    ms_ped = malloc(sizeof(struct mensagem_pedido));
 
	while(read(fd_entrada,ms_ped,sizeof(struct mensagem_pedido))){
       
       //falta ver generos diferentes
       
        sprintf(msg, "%d - %d - %lu - %d: %c - %d - RECEBIDO \n",i, getpid(), pthread_self(), ms_ped->pedido,ms_ped->gen,ms_ped->tempo);
    
        write(fd_registos, msg, strlen(msg));
        
        if(ms_ped->gen=='F'){
            nr_pedidos_global_F++;    
        }
        else{
            nr_pedidos_global_M++;
        }
       
        if(pthread_create(&tid[i], NULL, entrar_sauna, (void *)ms_ped)){
			perror("pthread_create_error");
			exit(1); 
		}
       
		i++;
   }
 
	for (int j=0;j<i;j++){
		if(pthread_join(tid[i],NULL)){
			perror("pthread_join_error");
			exit(1); 
		}
	}
  
    
    char estatistica[MAXL];
    sprintf(estatistica, " -Numero de pedidos Masculinos: %d \n -Numero de pedidos Femininos: %d \n -Numero de pedidos Total: %d \n -Numero de rejeicoes recebidas Masculinas: %d \n -Numero de rejeicoes recebidas Femininas: %d \n -Numero de rejeicoes recebidas no Total: %d \n -Numero de pedidos servidos Masculinos: %d \n -Numero de pedidos servidos Femininos: %d \n -Numero de pedidos servidos Total: %d", nr_pedidos_global_M,nr_pedidos_global_F, (nr_pedidos_global_F+nr_pedidos_global_M), nr_rejeitados_global_M,nr_rejeitados_global_F,(nr_rejeitados_global_M+nr_rejeitados_global_F),nr_servidos_global_M,nr_servidos_global_F,(nr_servidos_global_M+nr_servidos_global_F));
    
    write(fd_registos, estatistica, strlen(estatistica));
  

    unlink("/tmp/rejeitados");
    
    exit(0);
    
}

