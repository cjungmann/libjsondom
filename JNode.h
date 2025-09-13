/**
 * @file JNode.h
 * @brief Structs, enums, types, prototypes for JSON node
 */

#ifndef JMODEL_H
#define JMODEL_H

#include <stdbool.h>
#include <stddef.h>   // for NULL definition

/**
 * @brief Defined #JNode instance types
 *
 * These values not only identify the specific type of a given JNode
 * instance, but are also used as indexes into arrays of string and
 * function pointers for printing and destroying #JNode instances.
 */
typedef enum JDataType_e {
   DT_NULL,         ///< constant NULL/empty value
   DT_TRUE,         ///< constant true value
   DT_FALSE,        ///< constant false value
   DT_STRING,       ///< variable string value
   DT_INTEGER,      ///< variable long value
   DT_FLOAT,        ///< variable double value
   DT_ARRAY,        ///< collection of value nodes
   DT_PROPERTY,     /**< child of #DT_OBJECT that contains a #DT_STRING
                     * and a value node
                     */
   DT_OBJECT,       ///< collection of #DT_PROPERTY nodes
   DT_TYPE_LIMIT    ///< boundary value for testing valid #JDataType_e values
} JDataType;

/** typedef */
typedef struct JNode_s JNode;
/** typedef for destructor function pointer array */
typedef void (*JNode_payload_dtor)(JNode *node);
/** typedef for printer function pointer array */
typedef void (*JNode_printer)(const JNode *node, int indent);


// An item is a member of a collection, array or object

/**
 * @brief Memory representation of a JSON data element, including family links.
 *
 * The #JNode instance is mostly links to other #JNode instances, designed
 * to allow moving between nodes to specific relations.
 *
 * The #payload member is allocated separately according to the #JDataType
 * and the value of the instance.
 */
struct JNode_s {
   JNode     *parent;           ///<  node that counts @e this as a child
   JNode     *nextSibling;      ///< node that follows @e this
   JNode     *prevSibling;      ///< node that preceeds @e this
   JNode     *firstChild;       ///< first child node of @e this
   JNode     *lastChild;        /**< @brief last child, this pointer exists to speed-up
                                 * building the document memory model.
                                 */

   JDataType type;              ///< #JDataType identity member
   void      *payload;          ///< generic pointer to be cast according to the #type value.
};

/**
 * @defgroup AllFunctions All Functions
 * @{
 * @}
 */

/**
 * @ingroup AllFunctions
 * @defgroup Existential Functions that create or destroy JNodes.
 * @brief
 *    Functions that create and destroy JNode instances.
 * @{
 */
void JNode_emancipate(JNode *node);
void JNode_adopt(JNode *adoptee, JNode *parent, JNode *before);
bool JNode_create(JNode **new_node, JNode *parent, JNode *before);
void JNode_destroy(JNode **node);
bool JNode_discard_payload(JNode *node);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup NodeSetters Functions to prepare JNodes
 * @brief Functions for setting a node's #JDataType and JNode::payload
 * @{ 
 */
bool JNode_set_true(JNode *node);
bool JNode_set_false(JNode *node);
bool JNode_set_null(JNode *node);
bool JNode_set_integer(JNode *node, long value);
bool JNode_set_float(JNode *node, double value);
bool JNode_make_array(JNode *node);
bool JNode_make_object(JNode *node);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup StringSetters Functions that attach a string to a JNode
 * @{
 */
bool JNode_take_string(JNode *node, const char *str);
bool JNode_copy_string(JNode *node, const char *str);
/** @} */


/**
 * @ingroup AllFunctions
 * @defgroup CollectionAugmenters Functions that insert a childe into a JNode
 * @{
 */
bool JNode_make_null_property(JNode *node, const char *label);
bool JNode_array_insert_element(JNode *array, JNode *new_element, JNode *element_before);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup JNode_printers Functions for function pointer array
 * @brief Functions matching JNode_printer instance jNode_printers
 * @details
 *    Being aligned to the enum allows efficient indexed access to
 *    appropriate printer functions.
 * @{
 */
void JNode_print_null(const JNode *node, int indent);
void JNode_print_true(const JNode *node, int indent);
void JNode_print_false(const JNode *node, int indent);
void JNode_print_string(const JNode *node, int indent);
void JNode_print_integer(const JNode *node, int indent);
void JNode_print_float(const JNode *node, int indent);
void JNode_print_array(const JNode *node, int indent);
void JNode_print_property(const JNode *node, int indent);
void JNode_print_object(const JNode *node, int indent);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup TreePrinter Function to print JNode tree to stdout
 * @{
 */
void JNode_serialize(const JNode *node, int indent);
/** @} */




#endif
