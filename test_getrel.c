#include "jsondom.h"
#include <stdio.h>
#include <stdlib.h>   // for malloc/free
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
// #include <sys/stat.h> // for stat()
// #include <errno.h>    // for errno
// #include <string.h>   // for strerror()
// #include <stdint.h>   // for INTMAX_MAX define/constant
#include <stdbool.h>
#include <alloca.h>
#include <assert.h>

#define TEST_PATH "json_files/"

typedef void(*jsontest)(jd_Node *node);

void print_node_option(const jd_Node *node, int index, const char *name)
{
   printf("%d: %-12s: %s (%p)\n", index, name, (node?"available":"empty"), (void*)node);
}


void test_getrel(jd_Node *node)
{
   printf("You got to the test_getrel function.\n");
   jd_Node *parent = jd_get_relation(node, JD_PARENT);
   jd_Node *next_sib = jd_get_relation(node, JD_NEXT);
   jd_Node *first_child = jd_get_relation(node, JD_FIRST);
   jd_Node *prev_sib = jd_get_relation(node, JD_PREVIOUS);

   const char *tname = jd_id_name(node);

   printf("\033[2J\033[H");
   printf("Current node is a %s.\n", tname);

   print_node_option(parent, 0, "parent");
   print_node_option(next_sib, 1, "next_sib");
   print_node_option(first_child, 2, "first_child");
   print_node_option(prev_sib, 3, "prev_sib");

   const char *prompt = "Type an index (0-3) to move, 'q' to quit.";

   printf("\033[1G\033[2K");

   while (1)
   {
      printf(prompt);
      int ch = getchar();
      printf("\033[1G\033[2K");
      if (ch == 'q')
         break;
      else if (ch >='0' && ch <= '3')
      {
         jd_Node *rel = jd_get_relation(node, ch-'0');
         if (rel)
         {
            test_getrel(rel);
            break;
         }
         else
            printf("no relative, try again: %s", prompt);
      }
   }
}

void open_json_file(const char *filename, jsontest tfunc)
{
   int fd = open(filename, O_RDONLY);
   if (fd>=0)
   {
      jd_ParseError pe = {0};
      jd_Node       *node;

      if (jd_parse_file(fd, &node, &pe))
      {
         jd_serialize(STDOUT_FILENO, node);
         printf("\n\n");
         int ch = getchar();
         if (ch != 'q')
            (*tfunc)(node);

         jd_destroy(node);
      }
      else
         printf("Failed to parse '%s': '%s'\n", filename, pe.message);

      close(fd);
   }
   else
      printf("Failed to open file '%s'\n", filename);
}


int main(int argc, const char **argv)
{
   int retval = 0;
   const char *filename="json_files/good_object.json";
   open_json_file(filename, test_getrel);

   return retval;
}
