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

#define MAXL 4000
#define FIFO_ENTRADA "/tmp/entrada"
#define FIFO_REJEITADOS "/tmp/rejeitados"
#define MAX_READ_NULL_COUNTER 5


struct timeval before, after;
unsigned long long millisecondsBefore;


pthread_mutex_t sauna_line = PTHREAD_MUTEX_INITIALIZER; 
sem_t sauna_full;

int fd_registos;

int nr_pedidos_global_F=0;
int nr_rejeitados_global_F=0;
int nr_servidos_global_F=0;

int nr_pedidos_global_M=0;
int nr_rejeitados_global_M=0;
int nr_servidos_global_M=0;
char current_gen;
int nr_pessoas_sauna=0;

struct mensagem_pedido{
	int pedido;
	char gen;
	int tempo;
	int rejei;
};


/**
*@brief simula o uso da sauna por uma pessoa
*recebe um pedido como parâmetro atualiza o semáforo e todas as variaveis globais necessárias ao normal funcionamento do programa
*Faz sleep do tempo que uma pessoa deveria passar na sauna.
*/
void* entrar_sauna (void* arg);
/**
*@brief retorna o tempo atual em milisegundos
*/
unsigned long long getTime(struct timeval *time);
/**
*@brief abre todos os ficheiros necessários para o normal funcionamento do programa
*/
void openFiles(int *fd_entrada, int *fd_rejeitados, int *fd_registos);
/**
*@brief atualiza as variaveis globais nr_pedidos_global_F e nr_pedidos_global_M consoante o género
*/
void refreshNrPedidosGlobal(char gen);
/**
*@brief Simula o momento antes de entrada na sauna
*Atualiza o nº de pessoas que estão na sauna, cria a thread que simula o uso da sauna, e atualiza todas as variaveis globais necessárias
*/
void startThreadEntrarSauna(pthread_t *tid, struct mensagem_pedido *ms_ped);
/**
*@brief rejeita um pedido de entrada na sauna
*reenvia o pedido para o FIFO_REJEITADOS e atualiza todas as variaveis globais necessárias
*/ 
void rejeitaPedido(struct mensagem_pedido *ms_ped, int fd_rejeitados);
/**
*@brief Função usada para um final harmonioso do programa
*"rejeita" um ultimo pedido para o gerador saber que tem de terminar
*espera que todas as threads criadas pelo programa terminem e escreve as estatisticas no ficheiro de registos
*/
void endFunction(int fd_rejeitados, int thread_counter,pthread_t tid[]);


void endFunction(int fd_rejeitados, int thread_counter,pthread_t tid[]){
	struct mensagem_pedido end_ped;
	end_ped.gen='E';
	write(fd_rejeitados,&end_ped,sizeof(struct mensagem_pedido));

	for (int j=0;j<=thread_counter;j++){
		pthread_join(tid[j],NULL);

	}
	pthread_mutex_destroy(&sauna_line);

	char estatistica[MAXL];
	sprintf(estatistica, " -Numero de pedidos Masculinos: %d \n -Numero de pedidos Femininos: %d \n -Numero de pedidos Total: %d \n -Numero de rejeicoes recebidas Masculinas: %d \n -Numero de rejeicoes recebidas Femininas: %d \n -Numero de rejeicoes recebidas no Total: %d \n -Numero de pedidos servidos Masculinos: %d \n -Numero de pedidos servidos Femininos: %d \n -Numero de pedidos servidos Total: %d\n", nr_pedidos_global_M,nr_pedidos_global_F, (nr_pedidos_global_F+nr_pedidos_global_M), nr_rejeitados_global_M,nr_rejeitados_global_F,(nr_rejeitados_global_M+nr_rejeitados_global_F),nr_servidos_global_M,nr_servidos_global_F,(nr_servidos_global_M+nr_servidos_global_F));

	write(fd_registos, estatistica, strlen(estatistica));
}

void rejeitaPedido(struct mensagem_pedido *ms_ped, int fd_rejeitados){
	char msg[MAXL];

	if(ms_ped->gen=='F'){
		nr_rejeitados_global_F++;
	}
	else{
		nr_rejeitados_global_M++;
	}
	(*ms_ped).rejei+=1;

	struct timeval threadafter;
	unsigned long long millisecondsAfter = getTime(&threadafter);

	sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - REJEITADO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped->pedido,ms_ped->gen,ms_ped->tempo);
	write(fd_registos, msg, strlen(msg));
	write(fd_rejeitados,ms_ped,sizeof(struct mensagem_pedido));
}

unsigned long long getTime(struct timeval *time){
	gettimeofday(time, NULL);
	unsigned long long milliseconds = 1000000 * (unsigned long long)((*time).tv_sec) + (unsigned long long)((*time).tv_usec);
	return milliseconds;
}

void startThreadEntrarSauna(pthread_t *tid, struct mensagem_pedido *ms_ped){
	pthread_mutex_lock(&sauna_line);
	nr_pessoas_sauna++;    
	pthread_mutex_unlock(&sauna_line);	

	if(pthread_create(tid, NULL, entrar_sauna, (void *) ms_ped)){
		perror("pthread_create_error");
		exit(1);
	}
	if(ms_ped->gen=='F'){
		nr_servidos_global_F++;
	}
	else{
		nr_servidos_global_M++;
	}
	return;
}

void refreshNrPedidosGlobal(char gen){
	if(gen=='F'){
		nr_pedidos_global_F++;
	}
	else{
		nr_pedidos_global_M++;
	}
	return;
}

void openFiles(int *fd_entrada, int *fd_rejeitados, int *fd_registos){
	char regis[MAXL];
	sprintf(regis,"/tmp/bal.%d",getpid());

	(*fd_entrada) = open(FIFO_ENTRADA,O_RDONLY);
	(*fd_rejeitados) = open(FIFO_REJEITADOS,O_WRONLY);
	(*fd_registos) = open(regis, O_WRONLY | O_CREAT | O_EXCL, 0755);

	int flags = fcntl((*fd_entrada), F_GETFL, 0);
	fcntl((*fd_entrada), F_SETFL, flags | O_NONBLOCK);
	return;
}

void* entrar_sauna (void* arg){

	char msg[MAXL];
	struct mensagem_pedido *ms_ped = (struct mensagem_pedido*)arg;

	if(sem_wait(&sauna_full)){
		perror("sem_wait_error");
		exit(1);
	}

	usleep(ms_ped->tempo*1000.0);

	struct timeval threadafter;
	unsigned long long millisecondsAfter = getTime(&threadafter);

	sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - SERVIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped->pedido,ms_ped->gen,ms_ped->tempo);

	write(fd_registos, msg, strlen(msg));

	if(sem_post(&sauna_full)){
		perror("sem_post_error");
		exit(1);
	}

	pthread_mutex_lock(&sauna_line);
	nr_pessoas_sauna--;    
	pthread_mutex_unlock(&sauna_line);

	return NULL;
}

/**
*@brief Recebe todos os pedidos para entrar na sauna, caso o pedido tenha condições para entrar inicializa a sua simulação caso contrário envia para o FIFO_REJEITADOS
*/
int main(int argc, char **argv)
{
	millisecondsBefore = getTime(&before);

	if(sem_init(&sauna_full, 0, atoi(argv[1]))){
		perror("sem_init_error");
		exit(1);
	}

	int fd_entrada, fd_rejeitados, ms_ped_counter=0, thread_counter=0,read_null_counter=0;;
	pthread_t tid[MAXL];
	char msg[MAXL];

	openFiles(&fd_entrada,&fd_rejeitados,&fd_registos);

	struct mensagem_pedido ms_ped[MAXL];

	while(read(fd_entrada,&ms_ped[ms_ped_counter],sizeof(struct mensagem_pedido))){

		if(!(ms_ped[ms_ped_counter].gen=='F'||ms_ped[ms_ped_counter].gen=='M')){
			if(read_null_counter==MAX_READ_NULL_COUNTER)
				break;
			else {
				read_null_counter++;
				sleep(1);
				continue;
			}
		}
		else
			read_null_counter=0;

		unsigned long long millisecondsAfter = getTime(&after);
		sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - RECEBIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped[ms_ped_counter].pedido,ms_ped[ms_ped_counter].gen,ms_ped[ms_ped_counter].tempo);

		write(fd_registos, msg, strlen(msg));
		refreshNrPedidosGlobal(ms_ped[ms_ped_counter].gen);

		if(nr_pessoas_sauna==0){
			current_gen=ms_ped[ms_ped_counter].gen;
			startThreadEntrarSauna(&tid[thread_counter], &ms_ped[ms_ped_counter]);
			thread_counter++;
		}
		else {
			if(ms_ped[ms_ped_counter].gen==current_gen){
				startThreadEntrarSauna(&tid[thread_counter], &ms_ped[ms_ped_counter]);
				thread_counter++;
			}
			else{
				rejeitaPedido(&ms_ped[ms_ped_counter],fd_rejeitados);
			}
		}
		ms_ped_counter++;
	}

	endFunction(fd_rejeitados,thread_counter,tid);
	exit(0);
}