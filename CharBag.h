/**
 * @file CharBag.h
 * A CharBag is a container of a collection of characters that
 * constitutes a string, expanding to accommodate very long strings.
 */

#ifndef CHARBAG_H
#define CHARBAG_H

#include <stdbool.h>

#define CB_LEAF_SIZE 50

/** Simplified type */
typedef struct CharBag_s     CharBag;
/** Simplified type */
typedef struct CharBagLeaf_s CharBagLeaf;

/**
 * @brief Leaf of linked list of character buffers
 * Designed to be initialized with zeros (memset) to make
 * initializing a ::CharBag easier.
 */
struct CharBagLeaf_s {
   char buff[CB_LEAF_SIZE]; ///< memory to which characters are stored
   int index_next_char;     ///< index of next free space in #buff
   CharBagLeaf *next;       ///< pointer to next leaf if this one is full
};

/**
 * @brief Handle to managed linked list of char buffers.
 */
struct CharBag_s {
   CharBagLeaf rootLeaf;    ///< instance of first leaf of linked list
   CharBagLeaf *curLeaf;    ///< Last leaf into which new characters are stored
};

void initialize_CharBag(CharBag *charBag);
bool add_char_to_bag(CharBag *charBag, char char_to_save);
bool char_bag_to_string(CharBag *charBag, char **string_out);
void char_bag_cleanup(CharBag *charBag);

#endif
