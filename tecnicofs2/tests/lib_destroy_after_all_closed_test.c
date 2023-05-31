#include "fs/operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define COUNT 2
#define SIZE 512

/*  Simple test to check whether the implementation of
    tfs_destroy_after_all_closed is correct.
    Note: This test uses TecnicoFS as a library, not
    as a standalone server.
    We recommend trying more elaborate tests of tfs_destroy_after_all_closed.
    Also, we suggest trying out a similar test once the
    client-server version is ready (calling the tfs_shutdown_after_all_closed 
    operation).
*/


void *fn_thread(void *arg) {
    (void)
        arg; /* Since arg is not used, this line prevents a compiler warning */
    /* set *before* closing the file, so that it is set before
       tfs_destroy_after_all_close returns in the main thread
    */
    char *path = "/f1";

    char input[SIZE]; 
    memset(input, 'A', SIZE);

    char output [SIZE];

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
    return NULL;
}

int main() {

    assert(tfs_init() != -1);

    pthread_t threads[2];
    for (int a = 0; a<1;a++){
        assert(pthread_create(&threads[a], NULL, fn_thread, NULL) == 0);
    }
    //assert(tfs_destroy_after_all_closed() != -1); /*para testar mingua deixar esta linha*/
    for (int a = 0; a<1;a++){
        assert(pthread_join(threads[a], NULL) == 0);
    }
    assert(tfs_destroy_after_all_closed() != -1);/*para testar mingua tirar esta linha*/
    /*para testar mingua se der erro no open está certo*/
    printf("Successful test.\n");

    return 0;
}
 