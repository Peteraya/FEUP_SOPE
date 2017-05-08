all: sauna.c gerador.c 
	gcc  -Wall -o sauna sauna.c -lpthread -D_REENTRANT
	gcc  -Wall -o gerador gerador.c -lpthread -D_REENTRANT