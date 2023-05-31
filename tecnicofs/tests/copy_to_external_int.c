/*  *   *   *   *   *   *  *   *   *   *   *    *  *   *   *   *   *    *
 *                                                                      *
 *      MADE BY    :  Sofia Romeiro, ist198968, LETI                    *
 *                                                                      *
 *      BUG REPORT :  Karate Kid#9295 (Discord)                         *
 *                    sofiaromeiro23@tecnico.ulisboa.pt (webmail)       *
 *                                                                      *
 *  *   *   *   *   *   *  *   *   *   *   *    *  *   *   *   *   *    */

#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

int main() {

    int ints[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    char *path = "/f1";
    char *path2 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/test16.txt";;
    int to_read[40];

    assert(tfs_init() != -1);

    int file = tfs_open(path, TFS_O_CREAT);
    assert(file != -1);

    assert(tfs_write(file, ints, sizeof(ints)) != -1);

    assert(tfs_close(file) != -1);

    assert(tfs_copy_to_external_fs(path, path2) != -1);

    FILE *fp = fopen(path2, "r");

    assert(fp != NULL);

    assert(fread(to_read, sizeof(int), sizeof(ints), fp) == sizeof(ints) / sizeof(int));

    assert(memcmp(ints, to_read, sizeof(ints)) == 0);

    assert(fclose(fp) != -1);

    unlink(path2);

    printf("======> Successful test.\n\n");

    return 0;
}
