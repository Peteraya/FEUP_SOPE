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


//usar aqui mutex?
int nr_pedidos_global_F=0;
int nr_rejeitados_global_F=0;
int nr_descartados_global_F=0;

int nr_pedidos_global_M=0;
int nr_rejeitados_global_M=0;
int nr_descartados_global_M=0;




struct mensagem_pedido{
    int pedido;
    char gen;
    int tempo;
    int rejei;
};

struct criar_pedido{
    
    int nr_pedidos;
    int tempo;
    int fd_entrada;
    int fd_registos;
    
};



void* criador_pedidos(void* arg){
    
    struct criar_pedido *pedi = (struct criar_pedido*)arg;
    int nr_pedidos = pedi->nr_pedidos, max_uti = pedi->tempo, fd_entrada=pedi->fd_entrada, fd_registos=pedi->fd_registos;
    
    struct mensagem_pedido ms_ped;
    
    
    fd_entrada=open("/tmp/entrada",O_WRONLY);
    
    for(int i=0; i< nr_pedidos; i++){
        
        int luck=rand()%2;
        
        ms_ped.pedido = i;
        
        
        if(luck==0){
            ms_ped.gen='F';
            nr_pedidos_global_F++;    
        }
        else{
            ms_ped.gen='M';
            nr_pedidos_global_M++;
        }
        luck=rand() % max_uti+1;
        
        ms_ped.tempo=luck;
        ms_ped.rejei=0;
        char* msg;
        
        //?
        sprintf(msg, "%d - %d - %d: %c - %d - PEDIDO",i, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
        write(fd_registos, msg, sizeof(msg));
        
        
        write(fd_entrada,&ms_ped,sizeof(struct mensagem_pedido)); 
        
    }
    
    
    return NULL;
}


void* rejeita_pedidos(void* arg){
    
    struct mensagem_pedido ms_ped;
    
    
    int fd_rejeitados=open("/tmp/rejeitados",O_RDONLY);
    
    struct criar_pedido *pedi = (struct criar_pedido*)arg;
    int fd_registos = pedi->fd_registos;
    char* msg;
    
    
    
    read(fd_rejeitados,&ms_ped,sizeof(struct mensagem_pedido)); 
    
    sprintf(msg, "%d - %d - %d: %c - %d - REJEITADO",8, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
    write(fd_registos, msg, sizeof(msg));
    
     if(ms_ped.gen=='F'){
            nr_rejeitados_global_F++;    
        }
        else{
            nr_rejeitados_global_M++;
        }
    
    
    
    if(ms_ped.rejei<3){
        int fd_entrada=*(int*)arg; 
        ms_ped.rejei++;  //fazer aqui?
        
        
        if(ms_ped.gen=='F'){
            nr_pedidos_global_F++;    
        }
        else{
            nr_pedidos_global_M++;
        }
        
        sprintf(msg, "%d - %d - %d: %c - %d - PEDIDO",8, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
        write(fd_registos, msg, sizeof(msg));
        write(fd_entrada,&ms_ped,sizeof(struct mensagem_pedido)); 
    }
    else{
        
        if(ms_ped.gen=='F'){
            nr_descartados_global_F++;    
        }
        else{
           nr_descartados_global_M++;
        }
        sprintf(msg, "%d - %d - %d: %c - %d - DESCARTADO",8, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
        write(fd_registos, msg, sizeof(msg));
    }
    return NULL;
}

int main(int argc, char **argv)
{
    
    int fd_registos,fd_entrada;
    pthread_t tid_creat, tid_reject; 
    char* regis;
    
    
    sprintf(regis,"/tmp/ger.%d",getpid());
    
    struct criar_pedido *ped;
    
    fd_entrada = open("/tmp/entrada",O_WRONLY);
    fd_registos = open(regis, O_WRONLY | O_CREAT | O_EXCL, 0644);
    
    
    ped = malloc(sizeof(struct criar_pedido));
    (*ped).nr_pedidos = atoi(argv[1]);
    (*ped).tempo=atoi(argv[2]);
    (*ped).fd_entrada = fd_entrada;
    (*ped).fd_registos = fd_registos;
    
    
    if (mkfifo("/tmp/entrada",0660)<0){ 
        if (errno==EEXIST) 
            printf("FIFO '/tmp/entrada' already exists\n"); 
        else
            printf("Can't create FIFO\n"); 
    }
    
    
    pthread_create(&tid_creat, NULL, criador_pedidos, (void*)ped);
    
    pthread_create(&tid_reject, NULL, rejeita_pedidos, (void*)ped);
    
    pthread_join(tid_creat, NULL);
    pthread_join(tid_reject, NULL);
    
    char* estatistica;
    sprintf(estatistica, "-Numero de pedidos Masculinos: %d \n -Numero de pedidos Femininos: %d \n -Numero de pedidos Total: %d \n -Numero de rejeicoes recebidas Masculinas: %d \n -Numero de rejeicoes recebidas Femininas: %d \n -Numero de rejeicoes recebidas no Total: %d \n -Numero de rejeicoes descartadas Masculinas: %d \n, -Numero de rejeicoes descartadas Femininas: %d \n -Numero de rejeicoes descartadas no Total: %d \n", nr_pedidos_global_M,nr_pedidos_global_F, (nr_pedidos_global_F+nr_pedidos_global_M), nr_rejeitados_global_M,nr_rejeitados_global_F,(nr_rejeitados_global_M+nr_rejeitados_global_F),nr_descartados_global_M,nr_descartados_global_F,(nr_descartados_global_M+nr_descartados_global_F));
    
    write(fd_registos, estatistica, sizeof(estatistica));
    
    
    exit(0);
}
