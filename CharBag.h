#ifndef CHARBAG_H
#define CHARBAG_H

#include <stdbool.h>

#define CB_LEAF_SIZE 50

typedef struct CharBag_s     CharBag;
typedef struct CharBagLeaf_s CharBagLeaf;

// Can be initialized with memset to 0s:
struct CharBagLeaf_s {
   char buff[CB_LEAF_SIZE];
   int index_next_char;
   CharBagLeaf *next;
};

struct CharBag_s {
   CharBagLeaf rootLeaf;
   CharBagLeaf *curLeaf;
};

void initialize_CharBag(CharBag *charBag);
bool add_char_to_bag(CharBag *charBag, char char_to_save);
bool char_bag_to_string(CharBag *charBag, char **string_out);
void char_bag_cleanup(CharBag *charBag);

#endif
