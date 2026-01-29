/** @file JNode.c */

#include <stdlib.h>   // malloc/free
#include <string.h>   // memset
#include <stdio.h>    // printf
#include <assert.h>

#include "JNode.h"

/**
 * @brief Array of type names aligned to #JDataType enumeration.
 */
const char* JNode_TypeLabels[] = {
   "null",
   "true",
   "false",
   "string",
   "integer",
   "float",
   "array",
   "property",
   "object",
   "INVALID_TYPE"
};

/**
 * @brief Remove a #JNode instance from its family.
 *
 * Update links to @b node from its siblings and parent nodes,
 * replacing them with links to appropriate siblings.  Links to
 * children of @b node will be left intact.
 *
 * @param node Pointer to JNode instance to be removed from its family.
 */
void JNode_emancipate(JNode *node)
{
   if (node->parent)
   {
      // Remove/replace parent direct links to node
      if (node->parent->firstChild == node)
         node->parent->firstChild = node->nextSibling;
      if (node->parent->lastChild == node)
         node->parent->lastChild = node->prevSibling;
      // then remove node's link to parent
      node->parent = NULL;

      // Remove/replace sibling direct links to node
      if (node->nextSibling)
         node->nextSibling->prevSibling = node->prevSibling;
      if (node->prevSibling)
         node->prevSibling->nextSibling = node->nextSibling;
      // then remove node's links to siblings
      node->nextSibling = node->prevSibling = NULL;
   }
}

/**
 * @brief
 *    Incorporate @b adoptee node into children of @b parent.
 * @details
 *    The JNode @b adoptee will be added as a child of @b parent.
 *    The #JNode::parent pointer of @b adoptee will be set to
 *    @b parent.  Then, iff @b before is not NULL, @b adoptee will
 *    be inserted into the child list of @b parent, otherwise
 *    @b adoptee will be added after the last child of @b parent.
 *
 * @param adoptee   The JNode instance to be incorporated
 * @param parent    The JNode instance to use as the parent of @b adoptee
 * @param before    Optional JNode instance of a child of @b parent
 *                  after which @b adoptee will be placed
 */
void JNode_adopt(JNode *adoptee, JNode *parent, JNode *before)
{
   assert(parent && adoptee);

   // Previous family connections should have been severed:
   assert(adoptee->parent==NULL);
   assert(adoptee->prevSibling==NULL);
   assert(adoptee->nextSibling==NULL);

   adoptee->parent = parent;

   // Adjust all the links If inserting within children:
   if (before)
   {
      // Don't allow parental confusion or step-children
      assert(before->parent == parent);

      if (before->prevSibling)
      {
         adoptee->prevSibling = before->prevSibling;
         before->prevSibling->nextSibling = adoptee;
      }
      else
         // If no before->prevSibling, adoptee is the new firstChild
         parent->firstChild = adoptee;

      adoptee->nextSibling = before;
      before->prevSibling = adoptee;
   }
   else
   {
      if (parent->lastChild)
      {
         adoptee->prevSibling = parent->lastChild;
         parent->lastChild->nextSibling = adoptee;
         parent->lastChild = adoptee;
      }
      else
         parent->firstChild = parent->lastChild = adoptee;
   }
}


/**
 * @brief
 *    Returns a new JNode instance of type DT_NULL.
 * @details
 *    Uses @c malloc to create a new JNode instance, using
 *    #JNode_adopt to incorporate the new node into an existing
 *    family of nodes.
 *
 * @param new_node  Address of pointer to which the new JNode
 *                  instance will be copied
 * @param parent    The JNode instance to use as the parent of @b new_node
 * @param before    Optional JNode instance of a child of @b parent
 *                  after which @b new_node will be placed
 *
 * @return
 *    True if successful
 *    False if failed to get needed memory
 */
bool JNode_create(JNode **new_node, JNode *parent, JNode *before)
{
   JNode *node = (JNode*)malloc(sizeof(JNode));
   if (node)
   {
      memset(node, 0, sizeof(JNode));
      node->type = DT_NULL;

      // Adjust family relationships
      if (parent)
         JNode_adopt(node, parent, before);

      *new_node = node;
      return true;
   }

   return false;
}

/**
 * @brief
 *    Deletes JNode instance @b node after deleting everything
 *    to which it points.
 * @details
 *    Recursively deletes children, then siblings of @b node, then
 *    uses @c free to delete its payload (if appropriate) and finally,
 *    itself.
 *
 * @param node   Instance to be deleted after its pointers are freed.
 *
 * @warning
 *    Use with care: JNode_destroy clears the JNode pointer in the
 *    calling function.  Avoid attempting to free the memory twice.
 */
void JNode_destroy(JNode **node)
{
   if (*node)
   {
      // The child will destroy siblings, so we won't have to:
      if ((*node)->firstChild)
         JNode_destroy(&(*node)->firstChild);

      if ((*node)->nextSibling)
         JNode_destroy(&(*node)->nextSibling);

      if ((*node)->payload)
         free((void*)(*node)->payload);

      free(*node);
      *node = NULL;
   }
}

/**
 * @brief Intelligently free memory of payload member
 */
bool JNode_discard_payload(JNode *node)
{
   if (node->payload)
   {
      free((void*)node->payload);
      node->payload = NULL;
   }

   return true;
}

/**
 * @brief Safely converts an initialized JNode of any type to a DT_NULL JNode.
 * @param node   JNode to be converted
 * @return true for success, false for failure
 */
bool JNode_set_null(JNode *node)
{
   JNode_discard_payload(node);
   node->type = DT_NULL;
   return true;
}

/**
 * @brief Safely converts an initialized JNode of any type to a DT_TRUE JNode.
 * @param node   JNode to be converted
 * @return true for success, false for failure
 */
bool JNode_set_true(JNode *node)
{
   JNode_discard_payload(node);
   node->type = DT_TRUE;
   return true;
}

/**
 * @brief Safely converts an initialized JNode of any type to a DT_FALSE JNode.
 * @param node   JNode to be converted
 * @return true for success, false for failure
 */
bool JNode_set_false(JNode *node)
{
   JNode_discard_payload(node);
   node->type = DT_FALSE;
   return true;
}

/**
 * @brief Safely converts an initialized JNode of any type to a DT_INTEGER JNode.
 * @param node   JNode to be converted
 * @param value  value to be set in the payload
 * @return true for success, false for failure
 */
bool JNode_set_integer(JNode *node, const char *value)
{
   if (JNode_copy_string(node, value))
   {
      node->type = DT_INTEGER;
      return true;
   }

   return false;
}

/**
 * @brief Safely converts an initialized JNode of any type to a DT_FLOAT JNode.
 * @param node   JNode to be converted
 * @param value  value to be set in the payload
 * @return true for success, false for failure
 */
bool JNode_set_float(JNode *node, const char *value)
{
   if (JNode_copy_string(node, value))
   {
      node->type = DT_FLOAT;
      return true;
   }

   return false;
}

/**
 * @brief Use supplied string as node's payload.
 *
 * JNode_destroy will take responsibility for deleting
 * the string argument when the JNode is destroyed.
 */
bool JNode_take_string(JNode *node, const char *str)
{
   JNode_discard_payload(node);
   node->payload = (void*)str;

   node->type = DT_STRING;

   return true;
}

/**
 * @brief Allocate new payload memory into which 'str' will be copied
 */
bool JNode_copy_string(JNode *node, const char *str)
{
   JNode_discard_payload(node);
   int len = strlen(str);
   char *new_payload = (char*)malloc(len+1);
   if (new_payload)
   {
      memcpy(new_payload, str, len);
      new_payload[len] = '\0';
      node->payload = (void*)new_payload;

      node->type = DT_STRING;

      return true;
   }

   return false;
}

/**
 * @brief Discards all subordinate memory and values
 */
bool JNode_make_null_property(JNode *node, const char *label)
{
   JNode_discard_payload(node);
   if (node->firstChild)
      JNode_destroy(&(node->firstChild));
   node->type = DT_PROPERTY;

   JNode *label_node, *value_node;
   if (JNode_create(&label_node, node, NULL))
   {
      if (JNode_create(&value_node, node, NULL))
      {
         JNode_copy_string(label_node, label);
         JNode_set_null(value_node);

         return true;
      }
      else
         JNode_destroy(&(node->firstChild));
   }

   return false;
}

/**
 * @brief Safely converts an initialized JNode of any type to an empty DT_ARRAY JNode.
 * @param node   JNode to be converted
 * @return true for success, false for failure
 */
bool JNode_make_array(JNode *node)
{
   JNode_discard_payload(node);
   node->type = DT_ARRAY;

   return true;
}

/**
 * @brief Add an initialized JNode into an existing JNode array
 * @param array           Array into which the new JNode is to be inserted
 * @param new_element     element to be inserted
 * @param element_before  if not NULL, the new element will be placed before this element.
 * @return True for success, false for failure
 */
bool JNode_array_insert_element(JNode *array, JNode *new_element, JNode *element_before)
{
   assert(array->type == DT_ARRAY);
   JNode_adopt(new_element, array, element_before);
   return true;
}

/**
 * @brief Safely converts an initialized JNode of any type to an empty DT_OBJECT JNode.
 * @param node   JNode to be converted
 * @return true for success, false for failure
 */
bool JNode_make_object(JNode *node)
{
   JNode_discard_payload(node);
   node->type = DT_OBJECT;

   return true;
}

/**
 * @brief
 *    Array of pointers to JNode printer functions
 * @details
 *    This array of pointers is aligned to enum #JDataType.
 **/
JNode_printer jNode_printers[] = {
   JNode_print_null,
   JNode_print_true,
   JNode_print_false,
   JNode_print_string,
   JNode_print_integer,
   JNode_print_float,
   JNode_print_array,
   JNode_print_property,
   JNode_print_object
};

/**
 * @brief DT_NULL JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_null(const JNode *node, int indent)
{
   assert(node && node->type==DT_NULL);
   if (indent<0)
      printf("null");
   else
      printf("\n%*cnull", indent, ' ');
}

/**
 * @brief DT_TRUE JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_true(const JNode *node, int indent)
{
   assert(node && node->type==DT_TRUE);
   if (indent<0)
      printf("true");
   else
      printf("\n%*ctrue", indent, ' ');
}

/**
 * @brief DT_FALSE JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_false(const JNode *node, int indent)
{
   assert(node && node->type==DT_FALSE);
   if (indent<0)
      printf("false");
   else
      printf("\n%*cfalse", indent, ' ');
}

/**
 * @brief DT_STRING JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_string(const JNode *node, int indent)
{
   assert(node && node->type==DT_STRING);
   if (indent<0)
      printf("\"%s\"", (char*)node->payload);
   else
      printf("\n%*c\"%s\"", indent, ' ', (char*)node->payload);
}

/**
 * @brief DT_INTEGER JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_integer(const JNode *node, int indent)
{
   assert(node && node->type==DT_INTEGER);
   if (indent<0)
      printf("%s", (char*)node->payload);
   else
      printf("\n%*c%s", indent, ' ', (char*)node->payload);
}

/**
 * @brief DT_FLOAT JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_float(const JNode *node, int indent)
{
   assert(node && node->type==DT_FLOAT);
   if (indent<0)
      printf("%s", (char*)node->payload);
   else
      printf("\n%*c%s", indent, ' ', (char*)node->payload);
}

/**
 * @brief DT_ARRAY JNode printing function for jNode_printers array
 * @details Recursively-prints child nodes with incremented indent.
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_array(const JNode *node, int indent)
{
   assert(node && node->type==DT_ARRAY);

   JNode *child;
   int subindent = indent;
   if (indent<0)
      printf("[");
   else
   {
      printf("\n%*c[", indent, ' ');
      subindent += 4;
   }

   child = node->firstChild;
   while (child)
   {
      (*jNode_printers[child->type])(child, subindent);
      child = child->nextSibling;
      if (child)
         printf(",");
   }

   if (indent<0)
      printf("]");
   else
      printf("\n%*c]", indent, ' ');
}

/**
 * @brief DT_PROPERTY JNode printing function for jNode_printers array
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_property(const JNode *node, int indent)
{
   assert(node && node->type==DT_PROPERTY);
   assert(node->firstChild && node->firstChild->type == DT_STRING);
   assert(node->firstChild->nextSibling == node->lastChild);

   const char *label = (char*)node->firstChild->payload;
   JNode *value = node->lastChild;
   bool is_collection = value->type >= DT_ARRAY;

   if (indent < 0)
   {
      printf("\"%s\":",  label);
      (*jNode_printers[value->type])(value, indent);
   }
   else
   {
      printf("\n%*c\"%s\":", indent, ' ', label);
      if (is_collection)
         indent += 4;
      else
         indent = -1;   // print value on same line with indent value
      (*jNode_printers[value->type])(value, indent);
   }
}

/**
 * @brief DT_OBJECT JNode printing function for jNode_printers array
 * @details Recursively-prints child nodes with incremented indent.
 * @param node   JNode to be printed
 * @param indent multiple of indents to print before value
 */
void JNode_print_object(const JNode *node, int indent)
{
   assert(node && node->type==DT_OBJECT);

   JNode *child;
   int subindent = indent;
   if (indent<0)
      printf("{");
   else
   {
      printf("\n%*c{", indent, ' ');
      subindent += 4;
   }

   child = node->firstChild;
   while (child)
   {
      (*jNode_printers[child->type])(child, subindent);
      child = child->nextSibling;
      if (child)
         printf(",");
   }

   if (indent<0)
      printf("}");
   else
      printf("\n%*c}", indent, ' ');
}

/**
 * @brief Recursive function prints tree to stdout
 */
void JNode_serialize(const JNode *node, int indent)
{
   (*jNode_printers[node->type])(node, indent);
   printf("\n");
}

#ifdef JNODE_MAIN

void populate_simple_array(JNode *parent)
{
   JNode *child;
   JNode_create(&child, parent, NULL);
   JNode_create(&child, parent, NULL);
   JNode_set_true(child);
   JNode_create(&child, parent, NULL);
   JNode_set_false(child);
   JNode_create(&child, parent, NULL);
   JNode_copy_string(child, "This is a string");
}

void populate_simple_object(JNode *parent)
{
   JNode *child;

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "one_array");
   populate_simple_array(child->lastChild);

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "two_true");
   JNode_set_true(child->lastChild);

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "three_false");
   JNode_set_false(child->lastChild);

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "four_string");
   JNode_copy_string(child->lastChild, "String value");

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "five_integer");
   JNode_set_integer(child->lastChild, "1000");

   JNode_create(&child, parent, NULL);
   JNode_make_null_property(child, "six_float");
   JNode_set_float(child->lastChild, "3.141592653589");
}

void test_array_of_arrays(void)
{
   JNode *root;
   if (JNode_create(&root, NULL, NULL))
   {
      JNode_make_array(root);

      JNode *array;
      JNode_create(&array, root, NULL);
      JNode_make_array(array);
      populate_simple_array(array);

      JNode_create(&array, root, NULL);
      JNode_make_array(array);
      populate_simple_array(array);

      JNode_create(&array, root, NULL);
      JNode_make_object(array);
      populate_simple_object(array);

      JNode_create(&array, root, NULL);
      JNode_make_array(array);
      populate_simple_array(array);

      JNode_serialize(root, 0);
      JNode_destroy(&root);
   }
}

int main(int argc, const char **argv)
{
   test_array_of_arrays();
   return 0;
}


#endif // JNODE_MAIN




/* Local Variables:               */
/* compile-command: "b=JNode;    \*/
/*   gcc -std=c99 -Wall -Werror  \*/
/*       -ggdb -pedantic         \*/
/*       -fsanitize=leak,address \*/
/*       -D${b^^}_MAIN           \*/
/*       -o $b ${b}.c"            */
/* End:                           */


