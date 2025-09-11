#ifndef JPARSER_H
#define JPARSER_H

#include "JNode.h"

typedef struct CollectionTools_s CollectionTools;

/** typedef member of CollectionTools */
typedef bool (*Is_End_Char)(char chr);
/** typedef member of CollectionTools */
typedef bool (*Coerce_Type)(JNode *node);
/** typedef member of CollectionTools */
typedef bool (*Read_Member)(int fh, JNode *parent, JNode **new_node, char first_char);

/**
 * @brief Small collection of tools for parsing JSON data
 * @details
 *    The function pointers in the struct are used by JParser
 *    to perform context-dependent parsing.  Once the context is
 *    determined, the appropriate CollectionTools instance will be
 *    used to test for completion (Is_End_Char), transform the
 *    in-process JNode to the appropriate type (Coerce_Type), or to
 *    to read properties for objects or elements for arrays
 *    (Read_Member).  This is somewhat more efficient but also
 *    makes the code easier to understand.
 */
struct CollectionTools_s {
   Is_End_Char    is_end_char;
   Coerce_Type    coerce_type;
   Read_Member    read_member;
};


/**
 * @ingroup AllFunctions
 */
bool JParser(int fh, JNode *parent, JNode **node, char first_char);


#endif
