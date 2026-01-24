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
#include "isJsonNumber.h"

void report_parse_error(jd_ParseError *pe, int fh, const char *message)
{
   off_t offset = lseek(fh, 0, SEEK_CUR);
   pe->char_loc = offset;
   pe->message = message;
}

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
bool Array_ReadMember(int           fh,
                      JNode         *parent,
                      JNode         **new_node,
                      char          first_char,
                      char          *end_signal,
                      jd_ParseError *pe)
{
   return JParser(fh, parent, new_node, first_char, end_signal, pe);
}

/**
 * @brief Instance of CollectionTools_s to use when parsing an array
 */
CollectionTools arrayTools = {
   Array_IsEndChar,
   Array_CoerceType,
   Array_ReadMember,
   ']'
};

/** Implementation of CollectionTools_s::Is_End_Char when processing an object */
bool Object_IsEndChar(char chr) { return chr == '}'; }
/** Implementation of CollectionTools_s::Coerce_type when processing an object */
bool Object_CoerceType(JNode *node) { return JNode_make_object(node); }
/** Implementation of CollectionTools_s::ReadMember when processing an object */
bool Object_ReadMember(int fh,
                       JNode         *parent,
                       JNode         **new_node,
                       char          first_char,
                       char          *end_signal,
                       jd_ParseError *pe)
{
   bool retval = false;

   // Must be initialized before possible jump to early_exit label:
   RSHandle rsh_label = { 0 };

   if (first_char != '"')
   {
      report_parse_error(pe, fh,
                         "labels must be double-quoted");
      // if ((*Report_Error)(
      //        fh,
      //        "error parsing object member: "
      //        "labels must be enclosed in double-quotes."))
         goto early_exit;
   }

   // Read the string that's queued-up:
   ReadStringInit(&rsh_label, first_char);
   if (JReadString(fh, &rsh_label, pe))
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
            if (JParser(fh, NULL, &value_node, chr, &temp_end_signal, pe))
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
            report_parse_error(pe, fh,
                               "colons must follow labels");
            // if ((*Report_Error)(
            //        fh,
            //        "error parsing object member: "
            //        "A colon is required after a member label."))
               goto early_exit;
            break;
         }
      } // while (bytes_read = read...)

      if (bytes_read == 0)
         report_parse_error(pe, fh, "unexpected end-of-file");

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
   Object_ReadMember,
   '}'
};

/**
 * @brief Create a JNode collection according to the assigned tools.
 *
 * This function will create an Array or Object from the subordinate
 * data using the struct of function pointers to test and read
 * according to the collection type being created.
 */
bool parse_collection(int             fh,
                      JNode           *parent,
                      CollectionTools *tools,
                      JNode           **node,
                      jd_ParseError   *pe)
{
   bool retval = false;
   bool needs_member = false;
   bool collection_terminated = false;


   ssize_t chars_read;
   char chr = '\0';

   JNode *new_node = NULL;
   if (JNode_create(&new_node, NULL, NULL))
   {
      if ((*tools->coerce_type)(new_node))
      {
         while ( (chars_read = read(fh, &chr, 1)) == 1 )
         {
            if (isspace(chr))
               continue;

            if (needs_member)
            {
               if (chr == ',')
               {
                  report_parse_error(pe, fh,
                                     "comma in collection without preceeding member");
                  goto early_exit;
               }
            }
            else
            {
               if ((*tools->is_end_char)(chr))
               {
                  if (needs_member)
                  {
                     report_parse_error(pe, fh,
                                        "collection prematurely terminated");
                     goto early_exit;
                  }
                  else
                     retval = true;

                  // Goto parent attachment if appropriate
                  break;
               }
               else if (chr==']' || chr=='}')
               {
                  report_parse_error(pe, fh,
                                     "incorrect end char for the collection type");
                  goto early_exit;
               }
               else if (chr == ',')
               {
                  if (new_node->firstChild == NULL)
                  {
                     report_parse_error(pe, fh,
                                        "comma in collection without preceeding member");
                     goto early_exit;
                  }
                  else
                  {
                     needs_member = true;
                     continue;
                  }
               }
               else if (new_node->firstChild != NULL)
               {
                  report_parse_error(pe, fh,
                                     "missing comma between collection members");
                  goto early_exit;
               }
            }

            char end_char = '\0';
            JNode *new_el = NULL;
            if ((*tools->read_member)(fh, new_node, &new_el, chr, &end_char, pe))
            {
               needs_member = false;

               if (end_char == ',')
                  needs_member = true;
               else if ((*tools->is_end_char)(end_char))
               {
                  retval = true;
                  // Goto parent attachment if appropriate
                  break;
               }
               else if ((end_char=='"' && chr=='"')
                        || end_char == '\0'
                        || isspace(end_char))
                  continue;
               else
               {
                  report_parse_error(pe, fh,
                                     "unexpected member end char");
                  goto early_exit;
               }
            }
            else
            {
               // read_member should already have reported the error that put us here
               goto early_exit;
            }

            // if (!needs_member)
            // {
            //    if (chr == ',')
            //    {
            //       needs_member = true;
            //       continue;
            //    }
            //    else if ((*tools->is_end_char)(chr))
            //    {
            //       collection_terminated = true;
            //       retval = true;
            //       break;
            //    }
            //    else if ( strchr("]}", chr) )
            //    {
            //       // We have a collection-ending char that was not approved by
            //       // the previous 'is_end_char' test, so it's the wrong
            //       // collection-ending char:
            //       report_parse_error(pe, fh,
            //                          "incorrect end char for the open collection");
            //       goto early_exit;
            //    }
            //    else if ( needs_comma )
            //    {
            //       needs_comma = 0;
            //       if (chr == ',')
            //       {
            //          needs_member = true;
            //          continue;
            //       }
            //       else
            //       {
            //          report_parse_error(pe, fh,
            //                             "missing comma between collection memebers");
            //          goto early_exit;
            //       }
            //    }
            //    else
            //       needs_member = true;
            // }

            // if (needs_member)
            // {
            //    if (chr==']' || chr=='}')
            //    {
            //       report_parse_error(pe, fh,
            //                          "collection prematurely terminated");
            //       goto early_exit;
            //    }

            //    char end_char = '\0';
            //    JNode *new_el = NULL;
            //    if ( ! (*tools->read_member)(fh, new_node, &new_el, chr, &end_char, pe))
            //    {
            //       // read_member should already have reported the error that put us here
            //       goto early_exit;
            //    }

            //    // If comma ended read_member, no further checking is required:
            //    if (end_char == ',')
            //    {
            //       needs_member = true;
            //       continue;
            //    }
            //    else
            //    {
            //       needs_member = false;
            //       needs_comma = 1;

            //       if ((*tools->is_end_char)(end_char))
            //       {
            //          collection_terminated = true;
            //          retval = true;
            //          break;
            //       }
            //       else if ((end_char=='"' && chr=='"')
            //                || end_char == '\0'
            //                || end_char == ','
            //                || isspace(end_char))
            //          continue;
            //       else
            //       {
            //          report_parse_error(pe, fh,
            //                             "unexpected termination character");
            //          goto early_exit;
            //       }
            //    }
            // }
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

   if ( !collection_terminated )
      report_parse_error(pe, fh, "unterminated collection");

  early_exit:

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
             JNode         *parent,
             JNode         **node,
             char          first_char,
             char          *end_signal,
             jd_ParseError *pe)
{
   bool retval = true;

   // Advance past any whitespace:
   ssize_t chars_read;
   char chr = first_char ? first_char : ' ';
   while (isspace(chr))
   {
      if ((chars_read = read(fh, &chr, 1)) == 0 )
      {
         report_parse_error(pe, fh, "unexpected end-of-file");
         return false;
      }
   }

   JNode *new_node = NULL;

   RSHandle rsh = { 0 };

   switch(chr)
   {
      case '[':
         if (! parse_collection(fh, parent, &arrayTools, &new_node, pe))
         {
            retval = false;
            goto early_exit;
         }

         break;

      case '{':
         if (! parse_collection(fh, parent, &objectTools, &new_node, pe))
         {
            retval = false;
            goto early_exit;
         }

         break;

      default:
         // Allocating resources from here, no more
         // 'early_exit' to avoid memory leaks:
         ReadStringInit(&rsh, chr);
         if (JReadString(fh, &rsh, pe))
         {
            if (end_signal)
               *end_signal = rsh.end_signal;

            JNode *temp_node;
            // Defer adoption by parent until successfully parsing child:
            if (JNode_create(&temp_node, NULL, NULL))
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
                  else  // unquoted values, integer, float, or syntax-error
                  {
                     bool isFloat;
                     if (isJsonNumber(rsh.string, &isFloat))
                     {
                        if (isFloat)
                           JNode_set_float(temp_node, rsh.string);
                        else
                           JNode_set_integer(temp_node, rsh.string);
                     }
                     else
                     {
                        report_parse_error(pe, fh,
                                           "values must be quoted unless a "
                                           "number or a keyword");
                        retval = false;
                     }
                  }
               } // if (chr='"') ; else

               if (retval)
               {
                  if (parent)
                     JNode_adopt(temp_node, parent, NULL);

                  new_node = temp_node;
               }
               else
                  JNode_destroy(&temp_node);
            } // if JNode_create
         }
         else // if JReadString failed:
         {
            retval = false;
            ReadStringDestroy(&rsh);
         }
         break;   // default:
   }  // switch

   // Ok not to check, if new_node is null, *node should be null as well.
   *node = new_node;

  early_exit:
   return retval;
}


/**
 * @brief Confirms only whitespace characters remain in file.
 *
 * Since the root object must be a singleton, this function
 * confirms that no additional content follows the current
 * file pointer.  js_parse_file calls this to confirm.
 *
 * @param fh   handle of JSON file
 * @return true if only whitespace remains;
 *         false returned at first non-whitespace character
 */
bool confirm_no_further_file_content(int fh)
{
   ssize_t chars_read;
   char chr = ' ';
   while ((chars_read = read(fh, &chr, 1))==1)
   {
      if (!isspace(chr))
         return false;
   }

   return true;
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
      jd_ParseError pe = {0};
      char end_char;
      JNode *root;
      if (JParser(fh, NULL, &root, 0, &end_char, &pe))
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
