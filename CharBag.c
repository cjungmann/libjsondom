#include "CharBag.h"

#include <assert.h>
#include <string.h> // memset
#include <stdlib.h> // malloc/free


void initialize_CharBag(CharBag *charBag)
{
   assert(charBag);
   memset(charBag, 0, sizeof(CharBag));
   charBag->curLeaf = &charBag->rootLeaf;
}

bool add_char_to_bag(CharBag *charBag, char char_to_save)
{
   assert(charBag);
   bool retval = true;
   CharBagLeaf *cbLeaf = charBag->curLeaf;

   // Make new leaf if current leaf is full
   if (cbLeaf->index_next_char >= CB_LEAF_SIZE)
   {
      cbLeaf->next = (CharBagLeaf*)malloc(sizeof(CharBagLeaf));
      if (!cbLeaf->next)
      {
         retval = false;
         goto early_exit;
      }

      // Update pointers in the leaf and the charBag:
      cbLeaf = cbLeaf->next;
      memset(cbLeaf, 0, sizeof(CharBagLeaf));
      charBag->curLeaf = cbLeaf;
   }

   cbLeaf->buff[cbLeaf->index_next_char] = char_to_save;
   ++cbLeaf->index_next_char;

  early_exit:

   return retval;
}

bool char_bag_to_string(CharBag *charBag, char **string_out)
{
   assert(charBag);

   bool retval = true;
   CharBagLeaf *curLeaf;
   int charCount = 0;

   // Pass #1 to measure space required
   curLeaf = &charBag->rootLeaf;
   while (curLeaf)
   {
      charCount += curLeaf->index_next_char;
      curLeaf = curLeaf->next;
   }

   char *buff = (char*)malloc(charCount + 1);
   if (!buff)
   {
      retval = false;
      goto early_exit;
   }

   // Pass #2 to copy string to buffer
   curLeaf = &charBag->rootLeaf;
   char *ptr = buff;
   while (curLeaf)
   {
      if (curLeaf->index_next_char)
      {
         memcpy(ptr, curLeaf->buff, curLeaf->index_next_char);
         ptr += curLeaf->index_next_char;
      }
      curLeaf = curLeaf->next;
   }

   // Terminate string with NULL if we calculated string length properly:
   assert(ptr == buff + charCount);
   *ptr = '\0';

   *string_out = buff;

  early_exit:
   return retval;
}

void char_bag_cleanup(CharBag *charBag)
{
   assert(charBag);

   CharBagLeaf *cbLeaf = &charBag->rootLeaf;
   if (cbLeaf->next)
   {
      // Assume charBag and the root CharBagLeaf is stack-allocated.
      // Delete malloced leaves, then initialize the root leaf and
      // LeafBag as empty.
      CharBagLeaf *toDelete = cbLeaf->next;
      memset(charBag, 0, sizeof(CharBag));

      // Loop to free all child leaves
      while (toDelete)
      {
         CharBagLeaf *holdNext = toDelete->next;
         free((void*)toDelete);
         toDelete = holdNext;
      }
   }
}

#ifdef CHARBAG_MAIN

#include <stdio.h>

void add_string_to_char_bag(CharBag *charBag, const char *str)
{
   while (*str)
   {
      add_char_to_bag(charBag, *str);
      ++str;
   }
}

int main(int argc, const char **argv)
{
   CharBag charBag;
   initialize_CharBag(&charBag);

   add_string_to_char_bag(&charBag, "string\n\n");

   add_string_to_char_bag(&charBag, "This is a short sentence.\n\n");

   add_string_to_char_bag(&charBag,
                          "This is a longer, medium-length sentence that\n"
                          "will take up more room.\n"
                          "\n"
      );

   add_string_to_char_bag(&charBag,
                          "This is a paragraph that should force CharBag to\n"
                          "allocate more leaves to accommodate its length.\n"
                          "You should see all these lines in the final output,\n"
                          "if I've done everything right.\n"
                          "\n"
      );

   char *buff = NULL;
   if (char_bag_to_string(&charBag, &buff))
      printf("This is the string:\n\n%s\n", buff);
   else
      printf("Oops, there was an error.\n");

   free(buff);
   char_bag_cleanup(&charBag);

   return 0;
}


#endif // CHARBAG_MAIN


/* Compile with syntax check only */
/* Local Variables:               */
/* compile-command: "b=CharBag;  \*/
/*   gcc -std=c99 -Wall -Werror  \*/
/*   -ggdb -pedantic             \*/
/*   -fsanitize=address          \*/
/*   -D${b^^}_MAIN -o $b ${b}.c" \*/
/* End:                           */

