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
#include <sys/time.h>
#include <time.h>

#define MAXL 4000
#define FIFO_ENTRADA "/tmp/entrada"
#define FIFO_REJEITADOS "/tmp/rejeitados"


pthread_mutex_t pedi_lock = PTHREAD_MUTEX_INITIALIZER; 

unsigned long long millisecondsBefore;

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
	int fd_registos;
	int fd_entrada;
};

/**
*@brief retorna o tempo atual em milisegundos
*/
unsigned long long getTime();
/**
*@brief simula a criação de pedidos para entrar numa sauna
*cria vários pedidos aleatórios e envia-os para o FIFO_ENTRADA para o programa sauna os receber
*/
void* criador_pedidos(void* arg);
/**
*@brief recebe todos os pedidos que a sauna rejeitou e caso tenham condições reenvia-os para o FIFO_ENTRADA para a sauna os tentar receber novamente
*/
void* rejeita_pedidos(void* arg);

void* criador_pedidos(void* arg){

	struct criar_pedido *pedi = (struct criar_pedido*)arg;
	int nr_pedidos = pedi->nr_pedidos, max_uti = pedi->tempo, fd_entrada=pedi->fd_entrada, fd_registos=pedi->fd_registos;

	struct mensagem_pedido ms_ped;
	time_t t;
	srand((unsigned) time(&t));

	for(int i=0; i< nr_pedidos; i++){

		int luck=rand()%2;

		ms_ped.pedido = i;


		if(luck==0){
			ms_ped.gen='F';
			pthread_mutex_lock(&pedi_lock);
			nr_pedidos_global_F++;    
			pthread_mutex_unlock(&pedi_lock);
		}
		else{
			ms_ped.gen='M';
			pthread_mutex_lock(&pedi_lock);
			nr_pedidos_global_M++;
			pthread_mutex_unlock(&pedi_lock);
		}
		luck=rand() % max_uti+1;

		ms_ped.tempo=luck;
		ms_ped.rejei=0;
		char msg[MAXL];

		unsigned long long millisecondsAfter = getTime();

		sprintf(msg, "%.2f - %d - %d: %c - %d - PEDIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
		write(fd_registos, msg, strlen(msg));


		write(fd_entrada,&ms_ped,sizeof(struct mensagem_pedido)); 

	}


	return NULL;
}


void* rejeita_pedidos(void* arg){

	struct mensagem_pedido ms_ped;
	int fd_rejeitados=open(FIFO_REJEITADOS,O_RDONLY);
	struct criar_pedido *pedi = (struct criar_pedido*)arg;
	int fd_registos = pedi->fd_registos;
	int fd_entrada=pedi->fd_entrada;
	char msg[MAXL];

	while(read(fd_rejeitados,&ms_ped,sizeof(struct mensagem_pedido))){ 

		if(ms_ped.gen=='F'){
			nr_rejeitados_global_F++;    
		}
		else if(ms_ped.gen=='M'){
			nr_rejeitados_global_M++;
		}
		else if(ms_ped.gen=='E')
			break;
		else continue;

		unsigned long long millisecondsAfter = getTime();
		sprintf(msg, "%.2f - %d - %d: %c - %d - REJEITADO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
		write(fd_registos, msg, strlen(msg));

		if(ms_ped.rejei<3){      
			if(ms_ped.gen=='F'){
				pthread_mutex_lock(&pedi_lock);
				nr_pedidos_global_F++;    
				pthread_mutex_unlock(&pedi_lock);
			}
			else{
				pthread_mutex_lock(&pedi_lock);
				nr_pedidos_global_M++;
				pthread_mutex_unlock(&pedi_lock);
			}
			millisecondsAfter=getTime();
			sprintf(msg, "%.2f - %d - %d: %c - %d - PEDIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
			write(fd_registos, msg, strlen(msg));
			write(fd_entrada,&ms_ped,sizeof(struct mensagem_pedido)); 
		}
		else{
			if(ms_ped.gen=='F'){
				nr_descartados_global_F++;    
			}
			else{
				nr_descartados_global_M++;
			}
			millisecondsAfter=getTime();
			sprintf(msg, "%.2f - %d - %d: %c - %d - DESCARTADO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid() ,ms_ped.pedido,ms_ped.gen,ms_ped.tempo);
			write(fd_registos, msg, strlen(msg));
		}
	}
	return NULL;
}

unsigned long long getTime(){
	struct timeval *time=malloc(sizeof(struct timeval));
	gettimeofday(time, NULL);
	unsigned long long milliseconds = 1000000 * (unsigned long long)((*time).tv_sec) + (unsigned long long)((*time).tv_usec);
	return milliseconds;
}

/**
* Alma do programa gerador responsável pela criação dos FIFOS e threads criador_pedidos e rejeita_pedidos 
* espera que estes terminem e completa harmoniasamente o programa 
*/
int main(int argc, char **argv)
{
	millisecondsBefore = getTime();

	pthread_t tid_creat, tid_reject; 
	char regis[MAXL];

	if (mkfifo(FIFO_ENTRADA,0666)<0){ 
		if (errno==EEXIST) 
			printf("FIFO '/tmp/entrada' already exists\n"); 
		else
			printf("Can't create FIFO\n"); 
	}

	if(mkfifo(FIFO_REJEITADOS,0666)<0){ 
		if (errno==EEXIST) 
			printf("FIFO '/tmp/rejeitados' already exists\n"); 
		else
			printf("Can't create FIFO\n"); 
	}

	sprintf(regis,"/tmp/ger.%d",getpid());

	struct criar_pedido *ped;

	int fd_registos = open(regis, O_WRONLY | O_CREAT | O_EXCL, 0755);
	int fd_entrada=open(FIFO_ENTRADA,O_WRONLY);


	ped = malloc(sizeof(struct criar_pedido));
	(*ped).nr_pedidos = atoi(argv[1]);
	(*ped).tempo=atoi(argv[2]);
	(*ped).fd_registos = fd_registos;
	(*ped).fd_entrada = fd_entrada;

	pthread_create(&tid_creat, NULL, criador_pedidos, (void*)ped);

	pthread_create(&tid_reject, NULL, rejeita_pedidos, (void*)ped);

	pthread_join(tid_creat, NULL);
	pthread_join(tid_reject, NULL);
	pthread_mutex_destroy(&pedi_lock);

	char estatistica[MAXL];
	sprintf(estatistica, " -Numero de pedidos Masculinos: %d \n -Numero de pedidos Femininos: %d \n -Numero de pedidos Total: %d \n -Numero de rejeicoes recebidas Masculinas: %d \n -Numero de rejeicoes recebidas Femininas: %d \n -Numero de rejeicoes recebidas no Total: %d \n -Numero de rejeicoes descartadas Masculinas: %d \n -Numero de rejeicoes descartadas Femininas: %d \n -Numero de rejeicoes descartadas no Total: %d", nr_pedidos_global_M,nr_pedidos_global_F, (nr_pedidos_global_F+nr_pedidos_global_M), nr_rejeitados_global_M,nr_rejeitados_global_F,(nr_rejeitados_global_M+nr_rejeitados_global_F),nr_descartados_global_M,nr_descartados_global_F,(nr_descartados_global_M+nr_descartados_global_F));

	write(fd_registos, estatistica, strlen(estatistica));

	unlink(FIFO_ENTRADA);
	unlink(FIFO_REJEITADOS);
	exit(0);
}