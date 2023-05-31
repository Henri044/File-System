#include "operations.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#define S 11
#define LOG(fmt) fprintf(stderr, "LOG\t%s:%d | "fmt"\n", __FILE__, __LINE__);

typedef struct{
    int fcli;
    char* buffer;
    bool pode_continuar;
} client;
bool pode_continuar_main;
pthread_cond_t variaveis_de_condicao[S];
pthread_mutex_t mutexes_de_condicao[S];
pthread_mutex_t main_mutex;
pthread_cond_t main_vc;
client array_clients[S];
static int sessions_active = 0;
int retorno_erro = -1;
int retorno_bem = 0;
int fserv;

int aux_tfs_mount() {
    int fcli;
    ssize_t n;
    char buffer_mount[41];
    int session_id;

    if ((n = read(fserv,buffer_mount,40*sizeof(char))) == -1){
        return -1;
    }
    buffer_mount[40] = '\0';

    if ((fcli = open (buffer_mount,O_WRONLY)) < 0){
        return -1;
    }

    for (int i = 0; i < S; i++) {
        if (array_clients[i].fcli == fcli){
            if ((n = write(fcli, &retorno_erro, sizeof(int))) == -1){
                return -1;
            }
            return -1;
        }
    }

    if (S <= sessions_active) {
        if ((n = write(fcli, &retorno_erro, sizeof(int))) == -1){
            return -1;
        }
        return -1;
    }

    for (int i = 0; i < S; i++) {
        if (array_clients[i].fcli == -1) {
            array_clients[i].fcli = fcli;
            session_id = i;
            break;
        }
    }

    sessions_active++;

    if ((n = write(fcli, &session_id, sizeof(int))) == -1){
        return -1;
    }
    return 0;
}

int aux_tfs_unmount(int session_id) {
    ssize_t n;

    if (sessions_active <= 0) {
        if ((n = write(array_clients[session_id].fcli, &retorno_erro, sizeof(int))) == -1){
            return -1;
        }
        return -1;
    }

    sessions_active--;

    array_clients[session_id].fcli = -1;

    if ((n = write(array_clients[session_id].fcli, &retorno_bem, sizeof(int))) == -1){
        return -1;
    }

    close(fserv);
    close(array_clients[session_id].fcli);
    return 0;
}

int aux_tfs_open(int session_id) {
    ssize_t n;
    char filename[41];
    int flags;

    memcpy(&filename,array_clients[session_id].buffer + sizeof(char) + sizeof(int), 40 * sizeof(char));

    filename[40] = '\0';
    memcpy(&flags,array_clients[session_id].buffer + sizeof(char) + sizeof(int) + 40 * sizeof(char), sizeof(int));

    int g = tfs_open(filename, flags);

    if ((n = write(array_clients[session_id].fcli, &g, sizeof(int))) == -1){
        return -1;
    }

    return 0;
}

int aux_tfs_close(int session_id) {
    ssize_t n;
    int fhandle;

    memcpy(&fhandle, array_clients[session_id].buffer + sizeof(char) + sizeof(int), sizeof(int));
    
    int g = tfs_close(fhandle);

    if ((n = write(array_clients[session_id].fcli, &g, sizeof(int))) == -1){
        return -1;
    }

    return 0;
}

int aux_tfs_write(int session_id) {
    ssize_t n;
    int fhandle;
    size_t len;

    memcpy(&fhandle, array_clients[session_id].buffer + sizeof(char) + sizeof(int), sizeof(int));

    memcpy(&len, array_clients[session_id].buffer + sizeof(char) + sizeof(int) + sizeof(int), sizeof(size_t));

    char buffer_write[len+1];

    memcpy(&buffer_write, array_clients[session_id].buffer + sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t), len);

    buffer_write[len] = '\0';

    ssize_t g = tfs_write(fhandle, buffer_write, len);
    if (g == -1) {
        if ((n = write(array_clients[session_id].fcli, &retorno_erro, sizeof(int))) == -1){
            return -1;
        }
    }

    else {
        if ((n = write(array_clients[session_id].fcli, &g, sizeof(ssize_t))) == -1){
            return -1;
        }
    } 
    return 0;
}

int aux_tfs_read(int session_id) {
    ssize_t n;
    int fhandle;
    size_t len;

    memcpy(&fhandle, array_clients[session_id].buffer + sizeof(char) + sizeof(int), sizeof(int));

    memcpy(&len, array_clients[session_id].buffer + sizeof(char) + sizeof(int) + sizeof(int), sizeof(size_t));

    char content[len];
    ssize_t g = tfs_read(fhandle, content, len);
    long unsigned int bytes_read = (long unsigned int)g;
    if (bytes_read > 0) {
        char *buffer_read = malloc(sizeof(int) + (bytes_read+1)*sizeof(char));
        memcpy(buffer_read, &bytes_read, sizeof(int));
        memcpy(buffer_read + sizeof(int), content, (bytes_read) * sizeof(char));
        if ((n = write(array_clients[session_id].fcli, buffer_read, sizeof(int) + (bytes_read+1)*sizeof(char))) == -1){
            free(buffer_read);
            return -1;
        } 
        free(buffer_read);
        return 0;
    }

    else {
        char *buffer_read = malloc(sizeof(int));
        memcpy(buffer_read, &bytes_read, sizeof(int));
        if ((n = write(array_clients[session_id].fcli, buffer_read, sizeof(int))) == -1){
            free(buffer_read);
            return -1;
        }   
        free(buffer_read);
        return 0;
    }
}

int tfs_shutdown_after_all_closed(int session_id) {
    ssize_t n;

    int g = tfs_destroy_after_all_closed();
    if ((n = write(array_clients[session_id].fcli, &g, sizeof(int))) == -1){
        return -1;
    }
    return 0;
}

void *thread_function(void *sid){
    int session_thread = *((int *)sid);
    
    while(1){
        pthread_mutex_lock(&mutexes_de_condicao[session_thread]);
        while(array_clients[session_thread].pode_continuar == false){
            pthread_cond_wait(&variaveis_de_condicao[session_thread],&mutexes_de_condicao[session_thread]);
        }
    
        int op_code;
        memcpy(&op_code, array_clients[session_thread].buffer , sizeof(int));;

        /*if (op_code == TFS_OP_CODE_MOUNT){
            aux_tfs_mount(fserv);

        }*/
        if (op_code == TFS_OP_CODE_UNMOUNT){
            aux_tfs_unmount(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_OPEN){
            aux_tfs_open(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_CLOSE){
            aux_tfs_close(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
            
        }
        else if (op_code == TFS_OP_CODE_WRITE){
            aux_tfs_write(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_READ){
            aux_tfs_read(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED){
            tfs_shutdown_after_all_closed(session_thread);
            pthread_mutex_unlock(&mutexes_de_condicao[session_thread]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = true;
            pthread_cond_signal(&main_vc);
            array_clients[session_thread].pode_continuar =false;
            pthread_mutex_unlock(&main_mutex);
            free(sid);
            return 0;
        }
    }
}

int main(int argc, char **argv) {
    size_t len;
    int a, sessionid, flags, fhandle;
    char buffer[1];
    char filename[41];
    pthread_t threads[S];

    if (pthread_mutex_init(&main_mutex, 0) != 0)
        return -1;
    if (pthread_cond_init(&main_vc, 0) != 0)
            return -1;
    for ( int i = 0; i<S;i++){
        if (pthread_mutex_init(&mutexes_de_condicao[i], 0) != 0)
            return -1;
        if (pthread_cond_init(&variaveis_de_condicao[i], 0) != 0)
            return -1;
    }

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return -1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    if ((tfs_init())<0){
        return -1;
    }
    for (int i = 0; i < S; i++) {
        array_clients[i].fcli = -1;
    }
    if ((unlink(pipename))<0 && errno != ENOENT){
        return -1;
    }
    if ((mkfifo(pipename,0777))<0){
        exit(-1);
    }
    for (a = 0; a<S; a++){
        int *arg = malloc(sizeof(*arg));
        *arg = a;
        if ((pthread_create(&threads[a],NULL,thread_function,arg))<0){
            printf("thread mal creada\n");
            exit(-1);
        }
    }

    if ((fserv = open (pipename,O_RDONLY)) < 0){
        printf("mal aberta\n");
        return -1;    
    }
    while(1){
        ssize_t n = read(fserv,buffer, sizeof(char));
        int op_code = (int)buffer[0];
        if (n == -1){
            printf("mal lido\n");
            return -1;
        }
        if (n == 0){
            close(fserv);
            if ((fserv = open (pipename,O_RDONLY)) < 0){
                return -1;    
            }
            continue;
        }

        if (op_code == TFS_OP_CODE_MOUNT){
            aux_tfs_mount(fserv);
        }
        else if (op_code == TFS_OP_CODE_UNMOUNT){            
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_UNMOUNT;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_OPEN){
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            if ((n = read(fserv,filename,40*sizeof(char))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }
            filename[40] = '\0';

            if ((n = read(fserv,&flags,sizeof(int))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int)+ 40 * sizeof(char) + sizeof(int));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_OPEN;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int),&filename, 41 * sizeof(char));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int) + 40 * sizeof(char),&flags, sizeof(int));              
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_CLOSE){
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            if ((n = read(fserv,&fhandle,sizeof(int))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int)+ sizeof(int));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_CLOSE;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int),&fhandle, sizeof(int));              
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_WRITE){
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            if ((n = read(fserv,&fhandle,sizeof(int))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }

            if ((n = read(fserv,&len,sizeof(size_t))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }

            char buffer_write[len+1];

            if ((n = read(fserv,buffer_write,len*sizeof(char))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }

            buffer_write[len] = '\0';
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t) + len * sizeof(char));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_WRITE;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int),&fhandle, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int) + sizeof(int),&len, sizeof(size_t));
            memcpy(array_clients[sessionid].buffer+ sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t),buffer_write, len * sizeof(char));              
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_READ){
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            if ((n = read(fserv,&fhandle,sizeof(int))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }

            if ((n = read(fserv,&len,sizeof(size_t))) == -1){
                if ((n = write(array_clients[sessionid].fcli, &retorno_erro, sizeof(int))) == -1){
                    return -1;
                }
                return -1;
            }
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t) + len * sizeof(char));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_READ;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int),&fhandle, sizeof(int));
            memcpy(array_clients[sessionid].buffer + sizeof(char) + sizeof(int) + sizeof(int),&len, sizeof(size_t));           
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            pthread_mutex_unlock(&main_mutex);
        }
        else if (op_code == TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED){
            if ((n = read(fserv,&sessionid,sizeof(int))) == -1){
                return -1;
            }
            array_clients[sessionid].buffer = malloc(sizeof(char) + sizeof(int));
            array_clients[sessionid].buffer[0] = TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED;
            memcpy(array_clients[sessionid].buffer + sizeof(char),&sessionid, sizeof(int));
            pthread_mutex_lock(&mutexes_de_condicao[sessionid]);
            array_clients[sessionid].pode_continuar = true;
            pthread_cond_signal(&variaveis_de_condicao[sessionid]);
            pthread_mutex_unlock(&mutexes_de_condicao[sessionid]);
            pthread_mutex_lock(&main_mutex);
            pode_continuar_main = false;
            while(pode_continuar_main == false){
                pthread_cond_wait(&main_vc,&main_mutex);
            }
            free(array_clients[sessionid].buffer);
            close(fserv);
            unlink(pipename);
            pthread_mutex_unlock(&main_mutex);
            return 0;
        }
        
    }
    
}