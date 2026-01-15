#include "jsondom.h"
#include <stdio.h>
#include <stdlib.h>   // for malloc/free
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <sys/stat.h> // for stat()
#include <errno.h>    // for errno
#include <string.h>   // for strerror()
#include <stdint.h>   // for INTMAX_MAX define/constant
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

bool parse_test_file(const char *filename)
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
      jd_ParseError pe = {0};
      jd_Node *node;
      if (jd_parse_file(fd, &node, &pe))
      {
         printf("Successfully parsed file!\n");

         // jd_serialize(0, node);
         jd_destroy(node);
         retval = true;
      }
      else
         printf("Failed to parse %s: %s.\n", filename, pe.message);
   }
   else
      printf("Failed to open '%s': %s.\n", file_path, strerror(errno));

   return retval;
}

void display_node(const jd_Node *node, int indent)
{
   const char *s_type = jd_id_name(node);
   jd_Type t_type = jd_id_type(node);
   printf("%*.*sType '%s' (%d)\n",
          indent, indent, "",
          s_type, t_type);
}

void test_node_tree(jd_Node *tree)
{
   printf("Got to the test_node_tree, baby.\n");
   display_node(tree, 4);
}

/**
 * @brief Called by @ref test_get_relations to display relation and its contents
 */
void display_relation(jd_Node *node, jd_Relation rel)
{
   const char *names[] = {
      "parent", "nextSibling", "prevSibling", "firstChild", "lastChild", NULL
   };
   printf( "For '%s', compare jd_get_relation and matched relation function:\n", names[rel]);

   jd_Node *(*rfunc[])(jd_Node *) = {
      parent, nextSibling, prevSibling, firstChild, lastChild
   };

   jd_Node *from_getrel = jd_get_relation(node, rel);
   jd_Node *from_func = rfunc[rel](node);

   if (from_getrel == from_func)
      printf("Results matched (%p)!\n", from_getrel);
   else
      printf("Results NOT MATCHED (%p vs %p).\n", from_getrel, from_func);
}

void test_get_relations(jd_Node *tree)
{
   for (int i=0; i<=JD_LAST; ++i)
      display_relation(tree, i);
}


bool test_individual_file(const char *filename)
{
   bool retval = false;

   printf("\n\nAbout to open file \033[32;1m%s\033[39;22m.\n",
          filename);

   int fd = open(filename, O_RDONLY);
   if (fd < 0)
   {
      printf("Failed to open file \033[33;31;1m%s\033[39;22m.\n",
             filename);
      retval = false;
   }
   else
   {
      jd_ParseError pe;
      jd_Node *node;
      if (jd_parse_file(fd, &node, &pe))
      {
         test_node_tree(node);
         test_get_relations(node);
         jd_destroy(node);
         retval = true;
      }
   }

   return retval;
}

int process_list_file(void)
{
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
            if ( ! parse_test_file(*ptr))
            {
               printf("\033[31;1mFailed to parse \033[32;1m%s\033[31;1m.\033[39;22m\n", *ptr);
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

int run_arg_as_filename(const char *filepath)
{
   int retval = 0;

   struct stat rstat = { 0 };
   if (stat(filepath, &rstat))
   {
      printf("Failed to stat file '%s': %s.\n",
             filepath,
             strerror(errno));

      retval = 1;
   }
   else
   {
      if (!test_individual_file(filepath))
         retval = 1;
   }

   return retval;
}

int main(int argc, const char **argv)
{
   int retval = 0;

   if (argc == 1)
   {
      printf("Running default action 'process_list_file'\n");
      printf("Press any key to begin.\n");
      getchar();
      retval = process_list_file();
   }
   else
   {
      printf("Running arg as filename (%s).\n", argv[1]);
      printf("Press any key to begin.\n");
      getchar();
      retval = run_arg_as_filename(argv[1]);
   }

   return retval;
}
