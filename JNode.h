#ifndef JMODEL_H
#define JMODEL_H

#include <stdbool.h>

typedef enum JDataType_e {
   DT_NULL,         // constant NULL/empty value
   DT_TRUE,         // constant true value
   DT_FALSE,        // constant false value
   DT_STRING,
   DT_INTEGER,
   DT_FLOAT,
   DT_ARRAY,        // collection of values
   DT_PROPERTY,     // special node with DT_STRING and value nodes
   DT_OBJECT,       // collection of properties (name:value)
   DT_TYPE_LIMIT    // Mark extent to test valid data types
} JDataType;

typedef struct JNode_s JNode;

typedef void (*JNode_payload_dtor)(JNode *node);

// An item is a member of a collection, array or object

struct JNode_s {
   JNode     *parent;
   JNode     *nextSibling;
   JNode     *prevSibling;
   JNode     *firstChild;
   JNode     *lastChild;

   JDataType type;
   void      *payload;
};

// Manipulate family relationships
void JNode_emancipate(JNode *node);
void JNode_adopt(JNode *adoptee, JNode *parent, JNode *before);

// Construct/Destruct
bool JNode_create(JNode **new_node, JNode *parent, JNode *before);
void JNode_destroy(JNode **node);

// Change types and values
bool JNode_discard_payload(JNode *node);

bool JNode_set_true(JNode *node);
bool JNode_set_false(JNode *node);
bool JNode_set_null(JNode *node);
bool JNode_set_integer(JNode *node, long value);
bool JNode_set_float(JNode *node, double value);

bool JNode_take_string(JNode *node, const char *str);
bool JNode_copy_string(JNode *node, const char *str);

bool JNode_make_null_property(JNode *node, const char *label);

bool JNode_make_array(JNode *node);
bool JNode_array_insert_element(JNode *array, JNode *new_element, JNode *element_before);

bool JNode_make_object(JNode *node);
bool JNode_object_insert_property(JNode *object, JNode *new_node, JNode *node_before);

// Node-printing functions by type:
void JNode_print_null(const JNode *node, int indent);
void JNode_print_true(const JNode *node, int indent);
void JNode_print_false(const JNode *node, int indent);
void JNode_print_string(const JNode *node, int indent);
void JNode_print_integer(const JNode *node, int indent);
void JNode_print_float(const JNode *node, int indent);
void JNode_print_array(const JNode *node, int indent);
void JNode_print_property(const JNode *node, int indent);
void JNode_print_object(const JNode *node, int indent);

typedef void(*JNode_printer)(const JNode *node, int indent);

void JNode_serialize(const JNode *node, int indent);



#endif
