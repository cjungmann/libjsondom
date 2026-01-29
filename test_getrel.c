/**
 * @file test_getrel.c
 * @brief Rough test of navigation and value-printing functions.
 *
 * This simple test program was used to confirm proper execution
 * of the new *jd_get_relation* function, and other new functions,
 * *jd_node_value_length* and *jd_node_value* were also developed
 * to help identify the current location in order to confirm one's
 * position in the document.
 *
 * To build the executable, **getrel**, type the following at the
 * command line:
 *    make test
 *
 * The Makefile compiles source files with a *test_* prefix to an
 * executable named after the string following *test_*.
 *
 * Prerequisites:
 *   Besides the standard prerequisites for the jsondom library,
 *   this utility makes use of my *contools* library:
 *   https://github.com/cjungmann/libcontools
 */
#include "jsondom.h"
#include <stdio.h>
#include <stdlib.h>   // for malloc/free
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <contools.h>    // for get_keypress
#include <string.h>      // strcmp
#include <stdbool.h>
#include <alloca.h>
#include <assert.h>

#define TEST_PATH "json_files/"

/**
 * generic node processor to enable switching-out of the test.
 */
typedef void(*jsontest)(jd_Node *node);

/**
 * @brief Displays some node details to aid navigation
 */
void print_node_details(const jd_Node *node)
{
   if (node)
   {
      const char *type = jd_id_name(node);
      printf("Type:  \033[35;1m%s\033[39;22m\n", type);

      int len = jd_node_value_length(node);
      if (len)
      {
         char *buffer = (char*)malloc(len);
         if (buffer)
         {
            jd_node_value(node, buffer, len);
            printf("Value: \033[35;1m%s\033[39;22m\n", buffer);
            free(buffer);
         }
      }
   }
}

/**
 * Giving arrow key keystrings a name for better code comprehension
 */
#define KEYUP    "\033OA"
#define KEYDOWN  "\033OB"
#define KEYRIGHT "\033OC"
#define KEYLEFT  "\033OD"


/**
 * @brief An implementation of @ref jsontest function pointer.
 */
void test_getrel(jd_Node *node)
{
   char key_buff[10];

   while(1)
   {
      // Collect pointers to kin:
      jd_Node *parent = jd_get_relation(node, JD_PARENT);
      jd_Node *next_sib = jd_get_relation(node, JD_NEXT);
      jd_Node *first_child = jd_get_relation(node, JD_FIRST);
      jd_Node *prev_sib = jd_get_relation(node, JD_PREVIOUS);

      // Build the display, starting with a freshing of the screen
      printf("\033[2J\033[H");

      const char *bgcol="\033[48;5;236m";
      const char *bgoff="\033[49m";
      printf("Context map: angles point to available nodes.\n");
      printf("   %s   %c   %s\n",  bgcol, (parent?'^':' '), bgoff );
      printf("   %s%c  *  %c%s\n", bgcol, (prev_sib?'<':' '), (next_sib?'>':' '), bgoff);
      printf("   %s   %c   %s\n",  bgcol, (first_child?'v':' '), bgoff);

      print_node_details(node);
      printf("\n");
      printf("Use the arrow keys to move; type 'q' to quit.\n");
      fflush(stdout);

      // Declare out of loop to use as sentry:
      jd_Node *rel = NULL;

      // Wait and respond to user's keypress
      while (1)
      {
         const char *keyp = get_keystroke(key_buff, sizeof(key_buff));
         if (0 == strcmp(keyp, "q"))
            break;  // leaving rel==NULL to trigger outer-loop break
         else if (0 == strcmp(keyp, KEYUP) && parent)
            rel = parent;
         else if (0 == strcmp(keyp, KEYRIGHT))
            rel = next_sib;
         else if (0 == strcmp(keyp, KEYDOWN))
            rel = first_child;
         else if (0 == strcmp(keyp, KEYLEFT))
            rel = prev_sib;

         if (rel != NULL)
         {
            node = rel;
            break;
         }
      }

      if (rel == NULL)
         break;
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
         printf("\nPress any key to start examining nodes.\n");
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

   // The first command line argument, if provided, will
   // furnish the path to the JSON file to parse.  Otherwise,
   // the default file will be parsed:
   const char *filename = "json_files/good_object.json";
   if (argc>1)
      filename = argv[1];

   open_json_file(filename, test_getrel);

   return retval;
}
