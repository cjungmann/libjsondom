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
   printf("at file position %ld, ", offset);
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
bool Array_ReadMember(int fh,
                      JNode *parent,
                      JNode **new_node,
                      char first_char,
                      char *end_signal)
{
   return JParser(fh, parent, new_node, first_char, end_signal);
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
bool Object_ReadMember(int fh,
                       JNode *parent,
                       JNode **new_node,
                       char first_char,
                       char *end_signal
   )
{
   bool retval = false;

   if (first_char != '"')
   {
      if ((*Report_Error)(
             fh,
             "error parsing object member: "
             "labels must be enclosed in double-quotes."))
         goto early_exit;
   }

   // Read the string that's queued-up:
   RSHandle rsh_label = { 0 };
   ReadStringInit(&rsh_label, first_char);
   if (JReadString(fh, &rsh_label))
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
            char temp_end_signal = 0;
            if (JParser(fh, NULL, &value_node, chr, &temp_end_signal))
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
                     JNode_take_string(label_node, StealReadString(&rsh_label));
                     JNode_adopt(value_node, prop_node, NULL);

                     *new_node = prop_node;
                     retval = true;

                     if (end_signal)
                        *end_signal = temp_end_signal;
                  }
                  else
                     JNode_destroy(&prop_node);
               }
            }

            // Success or failure, we break after reading past the colon:
            break;
         }
         else
         {
            // A non-colon character after label is an error
            if ((*Report_Error)(
                   fh,
                   "error parsing object member: "
                   "A colon is required after a member label."))
               goto early_exit;
            break;
         }
      } // while

   } //  if (JReadString())

  early_exit:
   // delete in case retrieved string still attached
   ReadStringDestroy(&rsh_label);

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
   if (JNode_create(&new_node, NULL, NULL))
   {
      if ((*tools->coerce_type)(new_node))
      {
         while ( (chars_read = read(fh, &chr, 1)) == 1 )
         {
            if (isspace(chr))
               continue;

            if ((*tools->is_end_char)(chr))
            {
               retval = true;
               break;
            }
            else if (chr == ',')
               continue;
            else
            {
               char end_char = '\0';
               JNode *new_el = NULL;
               if ( ! (*tools->read_member)(fh, new_node, &new_el, chr, &end_char))
               {
                  if ((*Report_Error)(
                         fh,
                         "error parsing value starting with '%c' (%d).",
                         chr, chr))
                     break;
               }
               if ((*tools->is_end_char)(end_char))
               {
                  retval = true;
                  break;
               }
            }
         }

         if (retval)
         {
            if (parent)
               JNode_adopt(new_node, parent, NULL);

            *node = new_node;
            new_node = NULL;
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
 * @param first_char  character that introduces the current string
 * @param end_signal  pointer to which the string-end-signalling
 *                    character will be copied for later
 *                    consideration.
 * @return True if successful, false if failed
 */
bool JParser(int fh,
             JNode *parent,
             JNode **node,
             char first_char,
             char *end_signal)
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
            if (end_signal)
               *end_signal = rsh.end_signal;

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
