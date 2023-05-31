#include "client/tecnicofs_client_api.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*  This test is similar to test1.c from the 1st exercise.
    The main difference is that this one explores the
    client-server architecture of the 2nd exercise. */

int main(int argc, char **argv) {

    char *str = "AAA!";
    char *path = "/f1";
    char buffer[40];

    int f;
    ssize_t r;

    if (argc < 3) {
        printf("You must provide the following arguments: 'client_pipe_path "
               "server_pipe_path'\n");
        return 1;
    }
    printf("1\n");
    assert(tfs_mount(argv[1], argv[2]) == 0);
    f = tfs_open(path, TFS_O_CREAT);
    printf("2\n");
    assert(f != -1);
    r = tfs_write(f, str, strlen(str));
    printf("3\n");
    assert(r == strlen(str));
    printf("4\n");
    assert(tfs_close(f) != -1);
    f = tfs_open(path, 0);
    printf("5\n");
    assert(f != -1);
    r = tfs_read(f, buffer, sizeof(buffer) - 1);
    printf("6\n");
    assert(r == strlen(str));
    buffer[r] = '\0';
    printf("7\n");
    assert(strcmp(buffer, str) == 0);
    printf("8\n");
    assert(tfs_close(f) != -1);
    assert(tfs_shutdown_after_all_closed(f)==0);
    printf("9\n");
    //assert(tfs_unmount() == 0);

    printf("Successful test.\n");

    return 0;
}
