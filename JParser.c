/** @file JParser.c */

/** Enable usage of dprintf: */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>    // dprintf
#include <stdlib.h>   // malloc/free
#include <unistd.h>   // open/read/lseek
#include <ctype.h>    // isspace
#include <string.h>   // strerror
#include <fcntl.h>    // open()
#include <errno.h>    // errno for open()
#include <stdarg.h>   // for va_start, etc, for vprintf
#include <assert.h>

#include "JParser.h"
#include "JReadString.h"

bool Standard_Report_Error(int source_fh, const char *format, ...)
{
   off_t offset = lseek(source_fh, 0, SEEK_CUR);
   printf("at %ld, ", offset);
   va_list pargs;
   va_start(pargs, format);
   vprintf(format, pargs);
   va_end(pargs);
   printf("\n");
   return true;   // default exit after any error
}

Error_Reporter Report_Error = Standard_Report_Error;

/** Implementation of CollectionTools_s::Is_End_Char when processing an array */
bool Array_IsEndChar(char chr) { return chr == ']'; }
/** Implementation of CollectionTools_s::Coerce_type when processing an array */
bool Array_CoerceType(JNode *node) { return JNode_make_array(node); }
/** Implementation of CollectionTools_s::ReadMember when processing an array */
bool Array_ReadMember(int fh, JNode *parent, JNode **new_node, char first_char)
{
   return JParser(fh, parent, new_node, first_char);
}

/**
 * @brief Instance of CollectionTools_s to use when parsing an array
 */
CollectionTools arrayTools = {
   Array_IsEndChar,
   Array_CoerceType,
   Array_ReadMember
};

/** Implementation of CollectionTools_s::Is_End_Char when processing an object */
bool Object_IsEndChar(char chr) { return chr == '}'; }
/** Implementation of CollectionTools_s::Coerce_type when processing an object */
bool Object_CoerceType(JNode *node) { return JNode_make_object(node); }
/** Implementation of CollectionTools_s::ReadMember when processing an object */
bool Object_ReadMember(int fh, JNode *parent, JNode **new_node, char first_char)
{
   bool retval = false;

   // Read the string that's queued-up:
   RSHandle rsh = { 0 };
   ReadStringInit(&rsh, first_char);
   if (JReadString(fh, &rsh))
   {
      JNode *value_node = NULL;
      bool past_colon = false;
      char chr;
      ssize_t bytes_read;

      // Find colon, skip spaces, then read the value
      while ((bytes_read = read(fh, &chr, 1)) == 1)
      {
         if (isspace(chr))
            continue;
         else if (chr == ':')
            past_colon = true;
         else if (past_colon)
         {
            if (JParser(fh, NULL, &value_node, chr))
            {
               // We have the label string and value node,
               // so we can build the property now:
               JNode *prop_node = NULL;
               if (JNode_create(&prop_node, parent, NULL))
               {
                  prop_node->type = DT_PROPERTY;
                  JNode *label_node = NULL;
                  if (JNode_create(&label_node, prop_node, NULL))
                  {
                     JNode_take_string(label_node, StealReadString(&rsh));
                     JNode_adopt(value_node, prop_node, NULL);

                     *new_node = prop_node;
                     retval = true;
                  }
                  else
                     JNode_destroy(&prop_node);
               }
            }

            // Success or failure, we break after reading past the colon:
            break;
         }
         else
            break;
      } // while

      // Safely delete if not already done
      ReadStringDestroy(&rsh);
   } //  if (JReadString())

   return retval;
}

/**
 * @brief Instance of CollectionTools_s to use when parsing an object
 */
CollectionTools objectTools = {
   Object_IsEndChar,
   Object_CoerceType,
   Object_ReadMember
};

/**
 * @brief Create a JNode collection according to the assigned tools.
 *
 * This function will create an Array or Object from the subordinate
 * data using the struct of function pointers to test and read
 * according to the collection type being created.
 */
bool parse_collection(int fh, JNode *parent, CollectionTools *tools, JNode **node)
{
   bool retval = false;

   ssize_t chars_read;
   char chr;

   JNode *new_node = NULL;
   if (JNode_create(&new_node, parent, NULL))
   {
      if ((*tools->coerce_type)(new_node))
      {
         while ( (chars_read = read(fh, &chr, 1)) == 1 )
         {
            if (isspace(chr))
               continue;

            if ((*tools->is_end_char)(chr))
            {
               *node = new_node;
               new_node = NULL;
               retval = true;
               break;
            }
            else if (chr == ',')
               continue;
            else
            {
               JNode *new_el = NULL;
               if ( ! (*tools->read_member)(fh, new_node, &new_el, chr))
               {
                  if ((*Report_Error)(
                         fh,
                         "error parsing value starting with '%c' (%d).",
                         chr, chr))
                     break;
               }
            }
         }
      }
   }

   if (new_node)
      JNode_destroy(&new_node);

   return retval;
}


/**
 * @brief Create a JNode tree from a JSON document string.
 * @details
 *    Reads directly from a stream to create a Document Object
 *    Model (DOM) of a JSON document.
 *
 * @param fh          Handle to an open JSON file
 * @param parent      JNode under which new JNodes will be inserted
 * @param node        pointer to address of the newly-created JNode
 * @param first_char  Character that introduces the current string
 * @return True if successful, false if failed
 */
bool JParser(int fh, JNode *parent, JNode **node, char first_char)
{
   bool retval = true;

   // Advance past any whitespace:
   ssize_t chars_read;
   char chr = first_char ? first_char : ' ';
   while (isspace(chr))
   {
      if ((chars_read = read(fh, &chr, 1)) == 0 )
         return false;
   }

   JNode *new_node = NULL;

   RSHandle rsh = { 0 };

   switch(chr)
   {
      case '[':
         if (!parse_collection(fh, parent, &arrayTools, &new_node))
         {
            retval = false;
            goto early_exit;
         }
         break;

      case '{':
         if (!parse_collection(fh, parent, &objectTools, &new_node))
         {
            retval = false;
            goto early_exit;
         }
         break;

      default:
         ReadStringInit(&rsh, chr);
         if (JReadString(fh, &rsh))
         {
            JNode *temp_node;
            if (JNode_create(&temp_node, parent, NULL))
            {
               if (chr == '"')
                  JNode_take_string(temp_node, StealReadString(&rsh));
               else
               {
                  if ( 0 == strcmp(rsh.string, "null"))
                     JNode_set_null(temp_node);
                  else if ( 0 == strcmp(rsh.string, "true"))
                     JNode_set_true(temp_node);
                  else if ( 0 == strcmp(rsh.string, "false"))
                     JNode_set_false(temp_node);
                  else // numbers
                  {
                     errno = 0;
                     if (strchr(rsh.string, '.'))
                     {
                        double tval = strtod(rsh.string, NULL);
                        if (errno == 0)
                           JNode_set_float(temp_node, tval);
                        else
                        {
                           (*Report_Error)(
                                  fh,
                                  "Error converting '%s' to a double (%s).",
                                  rsh.string, strerror(errno));
                           retval = false;
                        }
                     }
                     else
                     {
                        long tval = strtol(rsh.string, NULL, 10);
                        if (errno == 0)
                           JNode_set_integer(temp_node, tval);
                        else
                        {
                           (*Report_Error)(
                              fh,
                              "Error converting '%s' to a long (%s).",
                              rsh.string, strerror(errno));
                           retval = false;
                        }
                     }
                  }
               } // if (chr='"') ; else

               if (retval)
                  new_node = temp_node;
               else
                  JNode_destroy(&temp_node);
            } // if JNode_create
         }
         ReadStringDestroy(&rsh);
         break;   // default:
   }  // switch

   // Ok not to check, if new_node is null, *node should be null as well.
   *node = new_node;

  early_exit:
   return retval;
}



#ifdef JPARSER_MAIN

#include "CharBag.c"
#include "JNode.c"
#include "JReadString.c"

int main(int argc, const char **argv)
{
   const char *filename="SIMPLE.json";
   if ( argc > 1 )
      filename = argv[1];

   int fh = open(filename, O_RDONLY);
   if (fh)
   {
      JNode *root;
      if (JParser(fh, NULL, &root, 0))
      {
         JNode_serialize(root, 0);
         JNode_destroy(&root);
      }

      close(fh);
   }

   return 0;
}



#endif   // JPARSER_MAIN

/* Local Variables:               */
/* compile-command: "b=JParser;  \*/
/*   gcc -std=c99 -Wall -Werror  \*/
/*       -ggdb -pedantic         \*/
/*       -fsanitize=leak,address \*/
/*       -D${b^^}_MAIN           \*/
/*       -o $b ${b}.c"            */
/* End:                           */
