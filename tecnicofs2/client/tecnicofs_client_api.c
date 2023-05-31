#include "tecnicofs_client_api.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

int fserv, fcli;
char const *client_name_path;
char const *server_name_path;
int session_id;

int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    ssize_t n;
    client_name_path = client_pipe_path;
    server_name_path = server_pipe_path;
    if ((unlink(client_name_path))<0 && errno != ENOENT){
        return -1;
    }
    if ((mkfifo(client_pipe_path,0777))<0)
        exit(-1);
    if ((fserv = open (server_pipe_path,O_WRONLY)) < 0) exit(-1);
    char *msg = malloc(41 * sizeof(char));
    msg[0] = TFS_OP_CODE_MOUNT;
    char buffer_pipename[40]; 
    memset(buffer_pipename, '\0',40);
    strcpy(buffer_pipename, client_name_path);
    memcpy(msg + sizeof(char),&buffer_pipename, 40 * sizeof(char));
    if ((n = write(fserv, msg, 41 * sizeof(char))) == -1){
        free(msg);
        return -1;
    }
    if ((fcli = open (client_pipe_path,O_RDONLY)) < 0) exit(-1);
    if ((n = read(fcli,&session_id,sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    free(msg);
    return 0;
    
}

int tfs_unmount() {
    int ack;
    ssize_t n;
    char *msg = malloc(sizeof(char) + sizeof(int));
    msg[0] = TFS_OP_CODE_UNMOUNT;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(int))) == -1){
        free(msg);
        return -1;
    }

    if ((n = close(fserv)) == -1){
        return -1;
    }
    if((n = close(fcli)) == -1){
        return -1;
    }
    if ((unlink(client_name_path))<0 && errno != ENOENT){
        return -1;
    }
    free(msg);
    return 0;
}

int tfs_open(char const *name, int flags) {
    int ack;
    ssize_t n;
    char name2[40];
    memset(name2, '\0',40);
    strcpy(name2, name);
    char *msg = malloc(sizeof(char) + sizeof(int) + 40 * sizeof(char) + sizeof(int));
    msg[0] = TFS_OP_CODE_OPEN;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int),&name2, 40 * sizeof(char));
    memcpy(msg + sizeof(char) + sizeof(int) + 40 * sizeof(char),&flags, sizeof(int));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int) + 40 * sizeof(char) + sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    free(msg);
    return 0;
}

int tfs_close(int fhandle) {
    int ack;
    ssize_t n;
    char *msg = malloc(sizeof(char) + sizeof(int) + sizeof(int));
    msg[0] = TFS_OP_CODE_CLOSE;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int),&fhandle, sizeof(int));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int) + sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    free(msg);
    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    ssize_t ack;
    ssize_t n;
    char *msg = malloc(sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t) + len * sizeof(char));
    msg[0] = TFS_OP_CODE_WRITE;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int),&fhandle, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int) + sizeof(int),&len, sizeof(size_t));
    memcpy(msg + sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t),buffer, len * sizeof(char));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t) + len * sizeof(char))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(ssize_t))) == -1){
        free(msg);
        return -1;
    }
    free(msg);
    return ack;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    long unsigned int ack;
    ssize_t n;
    char *msg = malloc(sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t));
    msg[0] = TFS_OP_CODE_READ;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int),&fhandle, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int) + sizeof(int),&len, sizeof(size_t));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int) + sizeof(int) + sizeof(size_t))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(int))) == -1){
        free(msg);
        return -1;
    }
    if ((n = read(fcli,buffer,ack)) == -1){
        free(msg);
        return -1;
    }
    free(msg);
    return (ssize_t)ack;
}

int tfs_shutdown_after_all_closed() {
    int ack;
    ssize_t n;
    char *msg = malloc(sizeof(char) + sizeof(int));
    msg[0] = TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED;
    memcpy(msg + sizeof(char),&session_id, sizeof(int));
    if ((n = write(fserv, msg, sizeof(char) + sizeof(int))) == -1){
        free(msg);
        printf("mal1\n");
        return -1;
    }
    if ((n = read(fcli,&ack,sizeof(int))) == -1){
        printf("mal2\n");
        free(msg);
        return -1;
    }
    return 0;
}
