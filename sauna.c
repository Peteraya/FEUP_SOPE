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



struct timeval before, after;
unsigned long long millisecondsBefore;



sem_t sauna_full;

int fd_registos;

int nr_pedidos_global_F=0;
int nr_rejeitados_global_F=0;
int nr_servidos_global_F=0;

int nr_pedidos_global_M=0;
int nr_rejeitados_global_M=0;
int nr_servidos_global_M=0;
char current_gen;

struct mensagem_pedido{
	int pedido;
	char gen;
	int tempo;
	int rejei;
};


void* entrar_sauna (void* arg){

    char msg[MAXL];
	struct mensagem_pedido *ms_ped = (struct mensagem_pedido*)arg;

	usleep(ms_ped->tempo*1000.0);

	gettimeofday(&after,NULL);
	unsigned long long millisecondsAfter = 1000000 * (unsigned long long)(after.tv_sec) + (unsigned long long)(after.tv_usec);
        
    sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - SERVIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped->pedido,ms_ped->gen,ms_ped->tempo);
        
	write(fd_registos, msg, strlen(msg));
        
    if(sem_post(&sauna_full)){
		perror("sem_post_error");
		exit(1); 
	}

	return NULL;
}


int main(int argc, char **argv)
{

	gettimeofday(&before, NULL);
	millisecondsBefore = 1000000 * (unsigned long long)(before.tv_sec) + (unsigned long long)(before.tv_usec);


	if(sem_init(&sauna_full, 0, atoi(argv[1]))){
		perror("sem_init_error");
		exit(1); 
	}


	int fd_entrada, fd_rejeitados, ms_ped_counter=0, thread_counter=0,read_null_counter=0;;
	pthread_t tid[MAXL];


	char regis[MAXL], msg[MAXL];

	sprintf(regis,"/tmp/bal.%d",getpid());

	fd_entrada = open("/tmp/entrada",O_RDONLY);

	fd_rejeitados = open("/tmp/rejeitados",O_WRONLY);

	fd_registos = open(regis, O_WRONLY | O_CREAT | O_EXCL, 0755);
	int flags = fcntl(fd_entrada, F_GETFL, 0);
	fcntl(fd_entrada, F_SETFL, flags | O_NONBLOCK);

	struct mensagem_pedido ms_ped[MAXL];

	while(read(fd_entrada,&ms_ped[ms_ped_counter],sizeof(struct mensagem_pedido))){

	if(!(ms_ped[ms_ped_counter].gen=='F'||ms_ped[ms_ped_counter].gen=='M')){
		if(read_null_counter==10)
			break;
		else {
			read_null_counter++;
			sleep(1);
			continue;
		}
	}
	else 
		read_null_counter=0;
		
gettimeofday(&after,NULL);
		unsigned long long millisecondsAfter = 1000000 * (unsigned long long)(after.tv_sec) + (unsigned long long)(after.tv_usec);

		sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - RECEBIDO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped[ms_ped_counter].pedido,ms_ped[ms_ped_counter].gen,ms_ped[ms_ped_counter].tempo);

		write(fd_registos, msg, strlen(msg));        
                
		if(ms_ped[ms_ped_counter].gen=='F'){
			nr_pedidos_global_F++;    
		}
		else{
			nr_pedidos_global_M++;
		}

		int val;

		if(sem_getvalue(&sauna_full,&val)){
			perror("sem_getvalue_error");
			exit(1); 
		}

		if(val==atoi(argv[1])){
			current_gen=ms_ped[ms_ped_counter].gen;
                        if(sem_wait(&sauna_full)){
				perror("sem_wait_error");
				exit(1); 
			}

			if(pthread_create(&tid[thread_counter], NULL, entrar_sauna, (void *) &ms_ped[ms_ped_counter])){
				perror("pthread_create_error");
				exit(1); 
			}
                      if(ms_ped[ms_ped_counter].gen=='F'){
				nr_servidos_global_F++;    
			}
			else{
				nr_servidos_global_M++;
			}
			thread_counter++;		
                }
		else {
			if(ms_ped[ms_ped_counter].gen==current_gen){
                                if(sem_wait(&sauna_full)){
					perror("sem_wait_error");
					exit(1); 
				}

				if(pthread_create(&tid[thread_counter], NULL, entrar_sauna, (void *) &ms_ped[ms_ped_counter])){
					perror("pthread_create_error");
					exit(1); 
				}
				
				if(ms_ped[ms_ped_counter].gen=='F'){
					nr_servidos_global_F++;    
				}
				else{
					nr_servidos_global_M++;
				}
				thread_counter++;
			}
			else{
				if(ms_ped[ms_ped_counter].gen=='F'){
					nr_rejeitados_global_F++;    
				}
				else{
					nr_rejeitados_global_M++;
				}
				ms_ped[ms_ped_counter].rejei+=1;
                                sprintf(msg, "%.2f - %d - %lu - %d: %c - %d - REJEITADO \n",(millisecondsAfter-millisecondsBefore)/1000.0, getpid(), pthread_self(), ms_ped[ms_ped_counter].pedido,ms_ped[ms_ped_counter].gen,ms_ped[ms_ped_counter].tempo);
				write(fd_registos, msg, strlen(msg));
				write(fd_rejeitados,&ms_ped[ms_ped_counter],sizeof(struct mensagem_pedido));
			}
		}
		ms_ped_counter++;
	}

struct mensagem_pedido end_ped;
end_ped.gen='E';
write(fd_rejeitados,&end_ped,sizeof(struct mensagem_pedido));

	for (int j=0;j<=thread_counter;j++){
		pthread_join(tid[j],NULL);

	}


	char estatistica[MAXL];
	sprintf(estatistica, " -Numero de pedidos Masculinos: %d \n -Numero de pedidos Femininos: %d \n -Numero de pedidos Total: %d \n -Numero de rejeicoes recebidas Masculinas: %d \n -Numero de rejeicoes recebidas Femininas: %d \n -Numero de rejeicoes recebidas no Total: %d \n -Numero de pedidos servidos Masculinos: %d \n -Numero de pedidos servidos Femininos: %d \n -Numero de pedidos servidos Total: %d\n", nr_pedidos_global_M,nr_pedidos_global_F, (nr_pedidos_global_F+nr_pedidos_global_M), nr_rejeitados_global_M,nr_rejeitados_global_F,(nr_rejeitados_global_M+nr_rejeitados_global_F),nr_servidos_global_M,nr_servidos_global_F,(nr_servidos_global_M+nr_servidos_global_F));

	write(fd_registos, estatistica, strlen(estatistica));
	exit(0);
}