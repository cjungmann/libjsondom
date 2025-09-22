#include "jsondom.h"
#include <stdio.h>
#include <stdlib.h>   // for malloc/free
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <sys/stat.h> // for stat()
#include <errno.h>    // for errno
#include <string.h>   // for strerror()
#include <stdbool.h>
#include <alloca.h>
#include <assert.h>

#define TEST_PATH "json_files/"


bool get_test_list(char **raw, const char*** index)
{
   const char *filepath= TEST_PATH "test.list";
   int fh = open(filepath, O_RDONLY);
   if (fh<0)
      printf("Error reading file '%s'.\n", filepath);
   else
   {
      struct stat lstat;
      if (fstat(fh, &lstat)!=0)
      {
         printf("Error getting the stat.\n");

         // Close and set handle to confirm close later
         close(fh);
         fh = -2;
      }
      else
      {
         off_t file_size = lstat.st_size;

         char **tindex = NULL;
         char *temp = (char*)malloc(file_size + 1);
         if (temp == NULL)
         {
            printf("Failed to get memory for files list.\n");

            // Close and set handle to confirm close later
            close(fh);
            fh = -2;
         }
         else
         {
            ssize_t bytes_read = read(fh, temp, file_size);
            temp[file_size] = '\0';

            // I'm gonna assume the read works, so only protecting with assert:
            assert(bytes_read == file_size);

            // Close and set handle to confirm close later
            close(fh);
            fh = -2;

            // Make two passes, first to count the lines in order
            // to make an array of pointers to each line, then to
            // populate the lines array with the strings
            int line_count = 0;

            // counting the lines
            char *ptr = temp;
            char *end = ptr + file_size;
            while (ptr < end)
            {
               if (*ptr == '\n')
               {
                  *ptr = '\0';
                  ++line_count;
               }

               ++ptr;
            }

            // Additional index pointer to set to NULL to
            // signal the end of the list
            ++line_count;

            int bufflen = line_count * sizeof(char*);
            char **tindex = (char**)malloc(bufflen);
            memset((void*)tindex, 0, bufflen);

            char **iptr = tindex;
            char **iend = iptr + line_count;
            ptr = temp;

            while (iptr < iend)
            {
               if (ptr >= end)
               {
                  *iptr = NULL;
                  break;
               }
               else
                  *iptr = ptr;

               // Skip ahead to next '\0'
               while (ptr < end && *ptr != 0)
                  ++ptr;

               // move past the '\0' for the next index
               ++ptr;

               ++iptr;
            }

            // Pass back to caller
            *raw = temp;
            *index = (const char**)tindex;

            // Set to NULL to prevent memory from being freed below
            temp = NULL;
            tindex = NULL;
         }

         // Delete memory allocated but not passed back to caller:
         if (temp)
         {
            free((void*)temp);
            if (tindex)
               free((void*)tindex);
         }

         // Confirm that file handle was deliberately closed:
         assert(fh == -2);
      }
   }

   return true;
}

bool test_file(const char *filename)
{
   bool retval = false;

   int name_len = strlen(filename);
   char *file_path = (char*)alloca(1 + name_len + strlen(TEST_PATH));
   strcpy(file_path, TEST_PATH);
   strcat(file_path, filename);

   printf("\n\n\033[32;1mAbout to open file '\033[34m%s\033[32m'.\033[39;22m\n", file_path);

   int fd = open(file_path, O_RDONLY);
   if (fd>=0)
   {
      jd_Node *node;
      if (jd_read(fd, &node))
      {
         printf("Successfully parsed file!\n");

         // jd_serialize(0, node);
         jd_destroy(node);
         retval = true;
      }
   }
   else
      printf("Failed to open '%s': %s.\n", file_path, strerror(errno));

   return retval;
}


int main(int argc, const char **argv)
{
   // Default indication: failure
   int retval = 0;

   char *raw = NULL;
   const char **index = NULL;
   if (get_test_list(&raw, &index))
   {
      // Succeeded getting file, at least
      retval = 0;

      const char **ptr = index;

      while (retval == 0 && *ptr)
      {
         if (**ptr != '#')
            if ( ! test_file(*ptr))
            {
               printf("Parsing \033[32;1m%s\033[39;22m failed.\n", *ptr);
               printf("Press 'q' to quit, anyother key to continue testing.\n");
               int keypressed = getchar();
               if (keypressed=='q' || keypressed=='Q')
               {
                  retval = 1;
                  break;
               }
            }

         ++ptr;
      }

      free(raw);
      free((void*)index);
   }

   return retval;
}
