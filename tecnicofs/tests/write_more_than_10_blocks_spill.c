/*  *   *   *   *   *   *  *   *   *   *   *    *  *   *   *   *   *    *
 *                                                                      *
 *      MADE BY    :  Sofia Romeiro, ist198968, LETI                    *
 *                                                                      *
 *      BUG REPORT :  Karate Kid#9295 (Discord)                         *
 *                    sofiaromeiro23@tecnico.ulisboa.pt (webmail)       *
 *                                                                      *
 *  *   *   *   *   *   *  *   *   *   *   *    *  *   *   *   *   *    */

#include "../fs/operations.h"
#include <assert.h>
#include <string.h>

#define COUNT 100
#define SIZE 404

int main() {

    char *path = "/f1";

    char input[SIZE]; 
    memset(input, 'A', SIZE);

    char output [SIZE];

    assert(tfs_init() != -1);

    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);
    for (int i = 0; i < COUNT; i++) {
        assert(tfs_write(fd, input, SIZE) == SIZE);
    }
    assert(tfs_close(fd) != -1);

    fd = tfs_open(path, 0);
    assert(fd != -1 );

    for (int i = 0; i < COUNT; i++) {
        assert(tfs_read(fd, output, SIZE) == SIZE);
        assert(memcmp(input, output, SIZE) == 0);
    }

    assert(tfs_close(fd) != -1);

    printf("======> Sucessful test\n\n");

    return 0;
}