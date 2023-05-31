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

    char big_str[SIZE_TO_TEST];
    size_t fr = 0, str_len = 0; 

    memset(big_str, '\0', sizeof(big_str));

    memset(big_str, 'b', sizeof(big_str) / 2);
    memset(big_str + (sizeof(big_str) / 2), 'u', (sizeof(big_str) / 2) - 1);

    char buffer[SIZE_TO_TEST+1];

    char *path = "/f1";
    char *path2 = "/home/martim/Desktop/SO_Todos/Projeto1/tecnicofs/test17.txt";  

    memset(buffer, '\0', sizeof(buffer));

    assert(tfs_init() != -1);

    int tfs_file = tfs_open(path, TFS_O_CREAT);
    assert(tfs_file != -1);

    assert(tfs_write(tfs_file, big_str, strlen(big_str)) == strlen(big_str));

    assert(tfs_close(tfs_file) != -1);

    assert(tfs_copy_to_external_fs(path, path2) != -1);

    FILE *fp = fopen(path2, "r");
    assert(fp != NULL);

    fr = fread(buffer, sizeof(char), strlen(big_str), fp);

    str_len = strlen(big_str);

    assert(fr == str_len);

    assert(strncmp(big_str, buffer, strlen(big_str)) == 0);
    assert(fclose(fp) != -1);

    unlink(path2);

    printf("======> Successful test.\n\n");

    return 0;
}
