#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#define P_READ 0
#define P_WRITE 1

typedef struct{
    char imprimido[50];
} Impressao;

typedef struct{
    Impressao fila[4];
} Queue;

Queue filaAtual;

int elementos_fila = 0;

pthread_mutex_t mutexFila;
pthread_cond_t filaCheia;
pthread_cond_t filaVazia;

void* add_printing(void* arg){
    while (1){
        char buffer[50];
        snprintf(buffer,sizeof(buffer),"This is a test!");
        pthread_mutex_lock(&mutexFila);

        while (elementos_fila >= 4){
            pthread_cond_wait(&filaCheia,&mutexFila);
        }

        elementos_fila++;

        Impressao to_print;
        strcpy(to_print.imprimido, buffer);
        filaAtual.fila[elementos_fila - 1] = to_print;

        if (elementos_fila > 0){
            pthread_cond_broadcast(&filaVazia);
        }

        pthread_mutex_unlock(&mutexFila);

        sleep(2);
    }

}

void* printing_work(void* arg){
    while (1){
        pthread_mutex_lock(&mutexFila);

        while (elementos_fila == 0){
            pthread_cond_wait(&filaVazia,&mutexFila);
        }

        Impressao to_print = filaAtual.fila[0];
        sleep(5);
        printf("Imprimindo: %s\n",to_print.imprimido);

        for (int i = 0; i < elementos_fila - 1; i++){
            filaAtual.fila[i] = filaAtual.fila[i + 1];
        }

        elementos_fila--;

        if (elementos_fila < 4){
            pthread_cond_broadcast(&filaCheia);
        }

        pthread_mutex_unlock(&mutexFila);
    }
}

int main() {
    pthread_t cliente[5];
    pthread_t impressora[3];

    pthread_mutex_init(&mutexFila,NULL);
    pthread_cond_init(&filaCheia,NULL);
    pthread_cond_init(&filaVazia,NULL);

    for (int i = 0; i < 5; i++){
        pthread_create(&cliente[i],NULL,add_printing,NULL);
    }

    for (int i = 0; i < 3; i++){
        pthread_create(&impressora[i],NULL,printing_work,NULL);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(cliente[i], NULL);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(impressora[i], NULL);
    }

    exit(EXIT_SUCCESS);
}