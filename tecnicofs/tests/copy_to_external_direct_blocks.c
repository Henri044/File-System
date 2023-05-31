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

#define SIZE_TO_TEST (1024*10)

int main() {

    char big_str[SIZE_TO_TEST+1];
    memset(big_str, 'x', sizeof(big_str));

    big_str[SIZE_TO_TEST] = '\0';

    char buffer[SIZE_TO_TEST];

    char *path = "/f1";
    char *path2 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/test9.txt";;  

    printf("SIZE TO TEST = %ld\n", sizeof(buffer));

    memset(buffer, '\0', sizeof(buffer));

    assert(tfs_init() != -1);

    int tfs_file = tfs_open(path, TFS_O_CREAT);
    assert(tfs_file != -1);

    assert(tfs_write(tfs_file, big_str, strlen(big_str)) == strlen(big_str));

    assert(tfs_close(tfs_file) != -1);

    assert(tfs_copy_to_external_fs(path, path2) != -1);

    FILE *fp = fopen(path2, "r");

    assert(fp != NULL);

    assert(fread(buffer, sizeof(char), strlen(big_str), fp) == strlen(big_str));

    assert(strncmp(big_str, buffer, strlen(big_str)) == 0);
    
    assert(fclose(fp) != -1);

    unlink(path2);

    printf("======> Successful test.\n\n");

    return 0;
}
