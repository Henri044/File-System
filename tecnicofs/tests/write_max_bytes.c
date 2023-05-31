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

#define SIZE (272384)

int main() {

   char *path = "/f1";
   int fd = 0;

   char input[SIZE]; 
   memset(input, 'R', SIZE);

   char output[SIZE];
   memset(output, '\0', SIZE);

   assert(tfs_init() != -1);

   fd = tfs_open(path, TFS_O_CREAT);

   assert(fd != -1);

   assert(tfs_write(fd, input, SIZE) == SIZE);

   assert(tfs_close(fd) != -1);

   fd = tfs_open(path,  0);                                            

   assert(fd != -1);

   assert(tfs_read(fd, output, SIZE) == SIZE);

   for (int i = 0; i < SIZE; i++) {
      if (input[i] != output[i]) {
         printf("(%d) input is %c and output is %c\n",i, input[i], output[i]);
         break;
      }
   }

   assert(memcmp(input, output, SIZE) == 0);

   assert(tfs_close(fd) != -1);

   printf("======> Sucessful test\n\n");

   return 0;
}