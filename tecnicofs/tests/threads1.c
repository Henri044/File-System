#include "../fs/operations.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*Primeiro teste de threads*/
#define COUNT 3
#define SIZE 3000
#define N 200 /*numero de threads*/

void* test1(void* arg) {

    char *path = "/f1";


    /* Writing this buffer multiple times to a file stored on 1KB blocks will 
       always hit a single block (since 1KB is a multiple of SIZE=256) */
    char input[SIZE]; 
    memset(input, 'A', SIZE);

    char output [SIZE];

    /* Write input COUNT times into a new file */
    int fd = tfs_open(path, TFS_O_CREAT);

    assert(fd != -1);
    for (int i = 0; i < COUNT; i++) {
        assert(tfs_write(fd, input, SIZE) == SIZE);
    }
 
    assert(tfs_close(fd) != -1);

    /* Open again to check if contents are as expected */
    fd = tfs_open(path, 0);

    assert(fd != -1 );
    for (int i = 0; i < COUNT; i++) {
        assert(tfs_read(fd, output, SIZE) == SIZE);
        assert (memcmp(input, output, SIZE) == 0);
    }
    assert(tfs_close(fd) != -1);

    printf("Successful thread\n");
    return arg;
}
int main(){
    int a;
    pthread_t threads[N];
    assert(tfs_init() != -1);
    for (a = 0; a<N; a++){
        if ((pthread_create(&threads[a],NULL,test1,NULL))==0){
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

    printf("Succesful test\n");
    exit(0);
}