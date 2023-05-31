#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*Terceiro teste de threads*/
#define COUNT 3
#define SIZE 3000
#define N 3 /*numero de threads*/

char names[N][30];
char *test_path1 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/threads1.txt";;
char *test_path2 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/threads2.txt";; 
char *test_path3 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/threads3.txt";  
char *path = "/f1";


void* test1(void *test_path) {
    assert(tfs_copy_to_external_fs(path, test_path) != -1);
    printf("Successful thread\n");
    return NULL;
}

int main(void){
    pthread_t threads[N];
    char *to_write = "AbC! aBc! ";
    
    assert(tfs_init() != -1);

    int file = tfs_open(path, TFS_O_CREAT);
    assert(file != -1);

    assert(tfs_write(file, to_write, strlen(to_write)) != -1);

    assert(tfs_close(file) != -1);

    if ((pthread_create(&threads[0],NULL,test1,test_path1))==0){
        printf("Criada tarefa1 %ld\n", threads[0]);
    }
    else{
        printf("Erro criação de tarefa\n");
        exit(1);
    }

    if ((pthread_create(&threads[1],NULL,test1,test_path2))==0){
        printf("Criada tarefa2 %ld\n", threads[1]);
    }
    else{
        printf("Erro criação de tarefa\n");
        exit(1);
    }

    if ((pthread_create(&threads[2],NULL,test1,test_path3))==0){
        printf("Criada tarefa3 %ld\n", threads[2]);
    }
    else{
        printf("Erro criação de tarefa\n");
        exit(1);
    }
    
    pthread_join(threads[0],NULL);
    pthread_join(threads[1],NULL);
    pthread_join(threads[2],NULL);

    assert(tfs_destroy() != -1);

    printf("Terminaram todas as threads\n");
    unlink(test_path1);
    unlink(test_path2);
    unlink(test_path3);
    exit(0);

    printf("Succesful test\n");
}