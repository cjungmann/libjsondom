#ifndef JSONDOM_H
#define JSONDOM_H

#include <stdbool.h>

typedef enum jd_Type_e {
   JD_NULL,         ///< constant NULL/empty value
   JD_TRUE,         ///< constant true value
   JD_FALSE,        ///< constant false value
   JD_STRING,       ///< variable string value
   JD_INTEGER,      ///< variable long value
   JD_FLOAT,        ///< variable double value
   JD_ARRAY,        ///< collection of value nodes
   JD_OBJECT        ///< collection of #JD_PROPERTY nodes
} jd_Type;

typedef struct jd_Node_s jd_Node;

/**
 * @brief Memory representation of a JSON data element, including family links.
 *
 * The #jd_Node instance is mostly links to other #jd_Node instances, designed
 * to allow moving between nodes to specific relations.
 *
 * The #payload member is allocated separately according to the #JDataType
 * and the value of the instance.
 */
struct jd_Node_s {
   jd_Node    *parent;          ///<  node that counts @e this as a child
   jd_Node    *nextSibling;     ///< node that follows @e this
   jd_Node    *firstChild;      ///< first child node of @e this
   jd_Node    *prevSibling;     ///< node that preceeds @e this
   jd_Node    *lastChild;       /**< @brief last child, this pointer exists to speed-up
                                * building the document memory model.
                                */

   jd_Type    type;             ///< #JDataType identity member
   void       *payload;         ///< generic pointer to be cast according to the #type value.
   const char *name;            ///< property name if needed, NULL value if not
};


typedef struct jd_ParseError_s {
   int        char_loc;      /**< offset in file of the character that
                              *   confirmed the detected error.  The actual
                              *   error may have occurred one or more places
                              *   earlier.
                              */
   const char *message;      ///< description of error
} jd_ParseError;

typedef struct jd_NodeSet {
   int     count;
   jd_Node **nodes;
} jd_NodeSet;

bool jd_parse_file(int fh, jd_Node **new_tree, jd_ParseError *pe);
void jd_destroy(jd_Node **node);

bool jd_search(jd_NodeSet *ns, const jd_Node *origin, const char *query);

void jd_serialize(int jd_out, const jd_Node *node, int indent);


#endif // JSONDOM_H
