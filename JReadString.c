/** @file JReadString.c */

#include "JReadString.h"
#include "CharBag.h"
#include "JParser.h"   // to access Report_Error function
#include <stdlib.h>    // malloc/free
#include <unistd.h>    // read
#include <string.h>    // strchr
#include <ctype.h>    // isspace
#include <assert.h>



/**
 * @brief Implementation of #RSEndCheck to use for a quoted string
 */
bool end_check_for_quoted(char chr)
{
   return chr=='"';
}

/**
 * @brief Implementation of #RSEndCheck to use for an unquoted string
 */
bool end_check_for_unquoted(char chr)
{
   return isspace(chr) || (strchr(",]}", chr) != NULL);
}

/**
 * @brief
 *    Prepare an uninitialized #RSHandle memory block
 *    to be used to read the next string in a stream
 * @param handle     Pointer to uninitialized RSHandle memory
 * @param firstChar  An already-read character that is the
 *                   first of the current string
 */
void ReadStringInit(RSHandle *handle, char firstChar)
{
   assert(handle);

   memset(handle, 0, sizeof(RSHandle));
   handle->first_char = firstChar;
   if (firstChar == '"')
      handle->end_check = end_check_for_quoted;
   else
      handle->end_check = end_check_for_unquoted;
}

/**
 * @brief Free all memory held by @b handle, then
 *        setting the pointers to NULL.
 * @param handle   The RSHandle object whose held memory
 *                 is to be freed
 */
void ReadStringDestroy(RSHandle *handle)
{
   assert(handle);
   if (handle->string)
   {
      free((void*)(handle->string));
      handle->string = NULL;
   }
}

/**
 * @brief
 *    Allows another function to take ownership
 *    of the string value by returning it and
 *    setting the pointer to it to NULL
 * @param handle   RSHandle that has collected a
 *                 string value
 * @return Pointer to collected string value
 */
const char *StealReadString(RSHandle *handle)
{
   assert(handle);
   const char *retval = handle->string;
   handle->string = NULL;
   return retval;
}

/**
 * @brief
 *    Read the rest of the current string into a memory block.
 * @details
 *    From the current location in the file, collect the
 *    characters that constitute the current string into
 *    a single memory block.
 *
 *    The end of unquoted strings is often indicated when
 *    the first character of the next string is recognized.
 *    JReadString will save the character that signaled the
 *    end of the current string in case it needs to be used
 *    as the beginning of the next string.  This allows for
 *    orderly progress on a single reading pass through the
 *    JSON contents.
 *
 * @param fh      handle to an open JSON document file
 * @param handle  pointer to an empty initialized RSHandle
 * @return True for success, false for failure
 */
bool JReadString(int fh, RSHandle *handle)
{
   bool retval = false;

   CharBag cbag;
   initialize_CharBag(&cbag);

   // Prepend the first char if it's not the string-introducing quote character:
   if (handle->first_char != '"')
      add_char_to_bag(&cbag, handle->first_char);

   bool escape_state = false;
   char chr;
   size_t bytes_read;
   while ((bytes_read = read(fh, &chr, 1)) == 1)
   {
      if (escape_state)
      {
         add_char_to_bag(&cbag, '\\');
         add_char_to_bag(&cbag, chr);
         escape_state = false;
      }
      else if (chr == '\\')
      {
         escape_state = true;
      }
      else if ((*handle->end_check)(chr))
      {
         char *value;
         if (char_bag_to_string(&cbag, &value))
         {
            handle->end_signal = chr;
            handle->string = value;
            retval = true;
            goto cleanup;
         }
      }
      else
         add_char_to_bag(&cbag, chr);
   }

   if (handle->first_char == '"')
   {
      // The loop above SHOULD have found a closing double-quote
      // and moved to cleanup label. Not finding the close quote
      // implies an incomplete document.  Leave retval==false
      // to terminate parsing.

      (*Report_Error)(fh, "Unexpected end-of-file while reading a string");
   }
   else
   {
      // I don't know why we might get here, so I'm gonna do
      // some testing in anticipation of removing this dead
      // code.
      char *value;
      if (char_bag_to_string(&cbag, &value))
      {
         // handle->end_signal = -1;
         handle->string = value;
         retval = true;
      }
   }

  cleanup:
   char_bag_cleanup(&cbag);

   return retval;
}


#ifdef JREADSTRING_MAIN

#include "CharBag.c"
#include <stdio.h>    // printf, remove
#include <fcntl.h>    // open/close
#include <errno.h>    // errno
#include <string.h>   // strerror()

#define TFILE "ReadString_tempfile"

bool test_string(const char *str)
{
   bool retval = true;

   // Declare in outer scope for Destroy
   RSHandle handle;
   memset(&handle, 0, sizeof(RSHandle));

   // Put string into file for reading
   int fh = open(TFILE, O_RDWR|O_CREAT|O_TRUNC);
   if (fh < 0)
   {
      printf("Open failure: '%s'\n", strerror(errno));
      retval = false;
      goto early_exit;
   }

   size_t bytes_written = write(fh, str, strlen(str));
   if (bytes_written != strlen(str))
   {
      printf("Write failure: '%s'\n", strerror(errno));
      retval = false;
      goto early_exit;
   }

   lseek(fh, 0, SEEK_SET);

   // Fake file established, simulate JParser processing:
   char chr;
   size_t bytes_read;

   while ((bytes_read = read(fh, &chr, 1)) == 1)
   {
      if (isspace(chr))
         continue;

      ReadStringInit(&handle, chr);
      if (JReadString(fh, &handle))
      {
         printf("The output is '%s'.\n", handle.string);
      }
      else
      {
         printf("JReadString failure: '%s'\n", strerror(errno));
         retval = false;
      }
      break;
   }

  early_exit:
   ReadStringDestroy(&handle);
   if (fh>0)
   {
      close(fh);
      remove(TFILE);
   }

   return retval;
}


int main(int argc, const char **argv)
{
   test_string("true, false ");
   test_string("null");
   test_string("\"Hi, mama\"");

   return 0;
}


#endif  // JREADSTRING_MAIN



/* Local Variables:                  */
/* compile-command: "b=JReadString; \*/
/*   gcc -std=c99 -Wall -Werror     \*/
/*       -ggdb -pedantic            \*/
/*       -fsanitize=leak,address    \*/
/*       -D${b^^}_MAIN              \*/
/*       -o $b ${b}.c"               */
/* End:                              */
