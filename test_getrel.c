/**
 * @file test_getrel.c
 * @brief Rough test of navigation and value-printing functions.
 *
 * I intend to abandon the jd_get_relation() function in favor of
 * directly accessing the relation pointer members of the jd_Node
 * object.
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
 * @defgroup LOOKUP Lookup tables for converting type values to descriptive strings
 * @{
 */
const char* DTYPES[] = {
   "NULL", "TRUE", "FALSE", "STRING", "INTEGER", "FLOAT", "ARRAY", "OBJECT"
};

/**
 * @brief representations of value based on jd_Node::type.
 *
 * A NULL value indicates that the jd_Node::payload is a string.
 * It's easier to test for NULL than to test index values of string
 * or number types that are stored as strings.
 */
const char *DVALUES[] = {
   "null", "true", "false", NULL, NULL, NULL, "*array*", "*object*"
};

/** @} */

/**
 * @brief Displays some node details to aid navigation
 */
void print_node_details(const jd_Node *node)
{
   if (node)
   {
      const char *type = DTYPES[node->type];

      const char *value = DVALUES[node->type];
      if (value==NULL)
         value = (char*)node->payload;

      const char *name = node->name;
      if (name==NULL)
         name = "n/a";

      printf("Type:  \033[35;1m%s\033[39;22m\n", type);
      printf("Value: \033[35;1m%s\033[39;22m\n", value);
      printf("Name:  \033[35;1m%s\033[39;22m\n", name);
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
      jd_Node *parent      = node->parent;
      jd_Node *next_sib    = node->nextSibling;
      jd_Node *first_child = node->firstChild;
      jd_Node *prev_sib    = node->prevSibling;

      // Build the display, starting with a freshing of the screen
      printf("\033[2J\033[H");

      const char *bgcol="\033[48;5;236m";
      const char *bgoff="\033[49m";
      printf("Context map: angles point to available nodes.\n");
      printf(bgcol);
      printf("    %c    \n", (parent?'^':' '));
      printf(" %c  *  %c \n", (prev_sib?'<':' '), (next_sib?'>':' '));
      printf("    %c    \n", (first_child?'v':' '));
      printf(bgoff);

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
         else if (0 == strcmp(keyp, KEYUP))
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
         // jd_serialize(STDOUT_FILENO, node, 0);
         // printf("\nPress any key to start examining nodes.\n");
         // int ch = getchar();
         // if (ch != 'q')
            (*tfunc)(node);

         jd_destroy(&node);
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
