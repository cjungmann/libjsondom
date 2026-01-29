#ifndef JSONDOM_H
#define JSONDOM_H

#include <stdbool.h>

typedef void jd_Node;

typedef struct jd_ParseError_s {
   int        char_loc;
   const char *message;
} jd_ParseError;


typedef enum jd_Type_e {
   JD_NULL,         ///< constant NULL/empty value
   JD_TRUE,         ///< constant true value
   JD_FALSE,        ///< constant false value
   JD_STRING,       ///< variable string value
   JD_INTEGER,      ///< variable long value
   JD_FLOAT,        ///< variable double value
   JD_ARRAY,        ///< collection of value nodes
   JD_PROPERTY,     /**< child of #JD_OBJECT that contains a #JD_STRING
                     * and a value node
                     */
   JD_OBJECT        ///< collection of #JD_PROPERTY nodes
} jd_Type;

/**
 * @brief Indexes of relations to a given jd_Node for use with
 *       jd_get_relation
 *
 * The values go up matching the direction of the relation in a
 * clockwise direction:
 * 
 * 0 up    (12:00) parent node
 * 1 right (03:00) next sibling
 * 2 down  (06:00) first child
 * 3 left  (09:00) previous sibling
 * also special index
 * 4 for pointer to last child (for appending new children)
 */
typedef enum jd_Relation_e {
   JD_PARENT = 0,
   JD_NEXT,
   JD_FIRST,
   JD_PREVIOUS,
   JD_LAST
} jd_Relation;


bool jd_parse_file(int fh, jd_Node **new_tree, jd_ParseError *pe);
void jd_destroy(jd_Node *node);

jd_Node* jd_get_relation(jd_Node *node, jd_Relation relation);

jd_Node* parent(jd_Node *node);
jd_Node* nextSibling(jd_Node *node);
jd_Node* firstChild(jd_Node *node);
jd_Node* prevSibling(jd_Node *node);
jd_Node* lastChild(jd_Node *node);

jd_Type jd_id_type(const jd_Node *node);
const char *jd_id_name(const jd_Node *node);
const void* jd_generic_value(const jd_Node *node);

int jd_get_value_length(const jd_Node *node);
int jd_stringify_value(const jd_Node *node, char *buffer, int bufflen);

int jd_stringify_null(const jd_Node *node, char *buffer, int bufflen);
int jd_stringify_true(const jd_Node *node, char *buffer, int bufflen);
int jd_stringify_false(const jd_Node *node, char *buffer, int bufflen);
int jd_stringify_integer(const jd_Node *node, char *buffer, int bufflen);

void jd_serialize(int jd_out, const jd_Node *node);


#endif // JSONDOM_H
