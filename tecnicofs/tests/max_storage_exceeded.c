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

#define SIZE (272385)

/**
   This test fills in a new file up to 20 blocks via multiple writes
   (therefore causing the file to hold 10 direct references + 10 indirect
   references from a reference block),
   each write always targeting only 1 block of the file, 
   then checks if the file contents are as expected
 */


int main() {

    char *path = "/f1";

    /* Writing this buffer multiple times to a file stored on 1KB blocks will 
       always hit a single block (since 1KB is a multiple of SIZE=256) */
    char input[SIZE]; 
    memset(input, 'R', SIZE);

    char output[SIZE];
    memset(output, '\0', SIZE);

    assert(tfs_init() != -1);

    /* Write input COUNT times into a new file */
    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);   

    assert(tfs_write(fd, input, SIZE) == SIZE-1);
    assert(tfs_close(fd) != -1);

    printf("======> Sucessful test\n\n");

    return 0;
}