/** @file CharBag.c */

#include "CharBag.h"

#include <assert.h>
#include <string.h> // memset
#include <stdlib.h> // malloc/free


/**
 * @brief Prepares a CharBag for use.
 *
 * Initializes unintialized memory.  Does not attempt to free
 * non-NULL members.
 *
 * @param[in,out] *charBag  Uninitialized ::CharBag instance
 */
void initialize_CharBag(CharBag *charBag)
{
   assert(charBag);
   memset(charBag, 0, sizeof(CharBag));
   charBag->curLeaf = &charBag->rootLeaf;
}

/**
 * @brief Add one character to the collection.
 *
 * @param[in] *charBag      CharBag instance accepting characters
 * @param[in] char_to_save  new character to save
 * @returns true for success, false for failure.  Failure will usually
 *          be a memory problem.
 */
bool add_char_to_bag(CharBag *charBag, char char_to_save)
{
   assert(charBag);
   bool retval = true;

   // leaf into which next char will be saved
   CharBagLeaf *leafForSaving = charBag->curLeaf;

   // if the leafForSaving is full, attach an empty new leaf for use:
   if (leafForSaving->index_next_char >= CB_LEAF_SIZE)
   {
      leafForSaving->next = (CharBagLeaf*)malloc(sizeof(CharBagLeaf));
      if (leafForSaving->next)
      {
         memset(leafForSaving->next, 0, sizeof(CharBagLeaf));

         // Update pointers in the leaf and the charBag:
         leafForSaving = leafForSaving->next;
         charBag->curLeaf = leafForSaving;
      }
      else
      {
         retval = false;
         goto early_exit;
      }
   }

   leafForSaving->buff[leafForSaving->index_next_char] = char_to_save;
   ++leafForSaving->index_next_char;

  early_exit:

   return retval;
}

/**
 * @brief Allocates single buffer to hold complete character collection.
 *
 * @param[in] *charBag       Character collection from which the new string
 *                           will be created and copied.
 * @param[out] **string_out  Pointer to address where the new string will
 *                           be returned.
 * @return true if @p string_out points to a new string, false if there is
 *         not enough memory to contain the characters.
 */
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

/**
 * @brief Deleted allocated memory of an initialized #CharBag.
 *
 * Recursively frees allocated memory after freeing any children.
 * Be careful not to call this function on an uninitiated #CharBag
 * whose pointer members will have nonsense addresses.
 *
 * @param[in] *charBag   pointer to redundant initialized #CharBag.
 */
void char_bag_cleanup(CharBag *charBag)
{
   assert(charBag);

   // Free all malloced leaves
   while (charBag->rootLeaf.next)
   {
      CharBagLeaf *leafToDelete = charBag->rootLeaf.next;
      charBag->rootLeaf.next = leafToDelete->next;

      // optional memset before free to mark deleted leaves:
      memset(leafToDelete, -1, sizeof(CharBagLeaf));

      free((void*)leafToDelete);
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

