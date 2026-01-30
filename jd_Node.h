/**
 * @file jd_Node.h
 * @brief Structs, enums, types, prototypes for JSON node
 */

#ifndef JMODEL_H
#define JMODEL_H

#include <stdbool.h>
#include <stddef.h>   // for NULL definition
#include "jsondom.h"

/**
 * @brief
 *    Integer error values for easier error reporting.
 * @details
 *    This is an error-handling alternative to the function
 *    pointer solution used while parsing.  It is easier to
 *    read or ignore according to development objectives.
 *
 *    This enumeration was created while developing stringify
 *    functions that must fail gracefully since they're meant
 *    to eventually  be used by the Bash builtin command.
 */
typedef enum jd_NodeError_e {
   JNE_SUCCESS = 0,       ///< 0 is success, as per convention
   JNE_FAILURE,           ///< generic unspecified error
   JNE_NULL_NODE,         ///< Forget to pass a node pointer
   JNE_INVALID_TYPE,      ///< Node type wrong for action to be taken
   JNE_OUT_OF_MEMORY,     ///< Not enough memory to complete action (like assignment)
   JNE_SMALL_BUFFER       ///< Buffer too small (or missing), especially for printing
} jd_NodeError;

/**
 * @brief Global variable defined in Stringify.c
 */
extern jd_NodeError jn_error;

/** typedef */
/** typedef for destructor function pointer array */
typedef void (*jd_Node_payload_dtor)(jd_Node *node);
/** typedef for printer function pointer array */
typedef void (*jd_Node_printer)(const jd_Node *node, int indent);


// An item is a member of a collection, array or object

/**
 * @defgroup AllFunctions All Functions
 * @{
 * @}
 */

/**
 * @ingroup AllFunctions
 * @defgroup Existential Functions that create or destroy jd_Nodes.
 * @brief
 *    Functions that create and destroy jd_Node instances.
 * @{
 */
void jd_Node_emancipate(jd_Node *node);
void jd_Node_adopt(jd_Node *adoptee, jd_Node *parent, jd_Node *before);
bool jd_Node_create(jd_Node **new_node, jd_Node *parent, jd_Node *before);
void jd_Node_destroy(jd_Node **node);
bool jd_Node_discard_payload(jd_Node *node);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup NodeSetters Functions to prepare jd_Nodes
 * @brief Functions for setting a node's #JDataType and jd_Node::payload
 * @{ 
 */
bool jd_Node_set_true(jd_Node *node);
bool jd_Node_set_false(jd_Node *node);
bool jd_Node_set_null(jd_Node *node);
bool jd_Node_set_integer(jd_Node *node, const char *value);
bool jd_Node_set_float(jd_Node *node, const char *value);
bool jd_Node_make_array(jd_Node *node);
bool jd_Node_make_object(jd_Node *node);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup StringSetters Functions that attach a string to a jd_Node
 * @{
 */
bool jd_Node_take_string(jd_Node *node, const char *str);
bool jd_Node_copy_string(jd_Node *node, const char *str);
/** @} */


/**
 * @ingroup AllFunctions
 * @defgroup CollectionAugmenters Functions that insert a childe into a jd_Node
 * @{
 */
bool jd_Node_make_null_property(jd_Node *node, const char *label);
bool jd_Node_array_insert_element(jd_Node *array, jd_Node *new_element, jd_Node *element_before);
/** @} */

/**
 * @ingroup AllFunctions
 * @defgroup jd_Node_printers Functions for function pointer array
 * @brief Functions matching jd_Node_printer instance jNode_printers
 * @details
 *    Being aligned to the enum allows efficient indexed access to
 *    appropriate printer functions.
 * @{
 */
void jd_Node_print_null(const jd_Node *node, int indent);
void jd_Node_print_true(const jd_Node *node, int indent);
void jd_Node_print_false(const jd_Node *node, int indent);
void jd_Node_print_string(const jd_Node *node, int indent);
void jd_Node_print_integer(const jd_Node *node, int indent);
void jd_Node_print_float(const jd_Node *node, int indent);
void jd_Node_print_array(const jd_Node *node, int indent);
void jd_Node_print_property(const jd_Node *node, int indent);
void jd_Node_print_object(const jd_Node *node, int indent);
/** @} */

int jd_Node_stringify_null(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_true(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_false(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_string(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_integer(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_float(const jd_Node *node, char *buffer, int bufflen);
int jd_Node_stringify_property(const jd_Node *node, char *buffer, int bufflen);

/**
 * @ingroup AllFunctions
 * @defgroup TreePrinter Function to print jd_Node tree to stdout
 * @{
 */
void jd_Node_serialize(const jd_Node *node, int indent);
/** @} */




#endif
