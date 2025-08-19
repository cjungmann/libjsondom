#ifndef JPARSER_H
#define JPARSER_H

#include "JNode.h"

typedef struct CollectionTools_s CollectionTools;

typedef bool (*Is_End_Char)(char chr);
typedef bool (*Coerce_Type)(JNode *node);
typedef bool (*Read_Member)(int fh, JNode *parent, JNode **new_node, char first_char);

struct CollectionTools_s {
   Is_End_Char    is_end_char;
   Coerce_Type    coerce_type;
   Read_Member    read_member;
};


bool JParser(int fh, JNode *parent, JNode **node, char first_char);


#endif
