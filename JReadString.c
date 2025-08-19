#include "JReadString.h"
#include "CharBag.h"
#include <stdlib.h>    // malloc/free
#include <unistd.h>    // read
#include <string.h>    // strchr
#include <assert.h>



bool end_check_for_quoted(char chr)
{
   return chr=='"';
}

bool end_check_for_unquoted(char chr)
{
   return isspace(chr) || (strchr(",]}", chr) != NULL);
}

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

void ReadStringDestroy(RSHandle *handle)
{
   assert(handle);
   if (handle->string)
   {
      free((void*)(handle->string));
      handle->string = NULL;
   }
}

const char *StealReadString(RSHandle *handle)
{
   assert(handle);
   const char *retval = handle->string;
   handle->string = NULL;
   return retval;
}

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

   if (handle->first_char != '"')
   {
      char *value;
      if (char_bag_to_string(&cbag, &value))
      {
         handle->end_signal = -1;
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
#include <ctype.h>    // isspace

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
