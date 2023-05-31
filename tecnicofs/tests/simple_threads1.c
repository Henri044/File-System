#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100000
#define N 3  /*numero de threads*/

char names[N][30];

void* test1(void *path) {

    char input[SIZE]; 
    memset(input, 'A', SIZE);

    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);
    tfs_write(fd, input, strlen(input));
    assert(tfs_close(fd) != -1);
    printf("Successful thread\n");
    return NULL;
}

int main(void){
    int a;
    pthread_t threads[N];
    for (a = 0; a<N; a++) {
        /*para testar meter sempre uma barra atrás exemplo: /f1 ou /f2*/
        printf("Inserir nome de ficheiro com uma barra atrás:");
        if ((scanf("%s", names[a])==1))
            printf("nice\n");
    }
    
    assert(tfs_init() != -1);

    for (a = 0; a<N; a++){
        if ((pthread_create(&threads[a],NULL,test1,names[a]))==0){
            printf("Criada tarefa %ld\n", threads[a]);
        }
        else{
            printf("Erro criação de tarefa\n");
            exit(1);
        }
    }
    for (a = 0; a<N; a++){
        pthread_join(threads[a],NULL);
    }

    assert(tfs_destroy() != -1);

    printf("Terminaram todas as threads\n");

    for (a = 0; a<N; a++){
    assert(tfs_lookup(names[a]));
    }

    printf("Succesful test\n");

    exit(0);
}