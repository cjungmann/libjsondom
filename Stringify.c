#include "JNode.h"
#include "JParser.h"  // for Report_Error function pointer
#include <string.h>   // for strlen, memcpy, etc
#include <stdlib.h>   // for labs

JNodeError jn_error = JNE_SUCCESS;

int JNode_stringify_generic(const JNode *node,
                            char *buffer,
                            int bufflen,
                            JDataType type,
                            const char *value)
{
   int retval = 0;

   if (node == NULL)
      jn_error = JNE_NULL_NODE;
   else if (node->type != type)
      jn_error = JNE_INVALID_TYPE;
   else
   {
      // No calling errors
      jn_error = JNE_SUCCESS;

      if (value)
      {
         retval = strlen(value) + 1;

         if (buffer && bufflen > 0)
         {
            int copylen;
            if (bufflen < retval)
               copylen = bufflen - 1;
            else
               copylen = retval;

            if (copylen > 1)
               memcpy(buffer, value, copylen);
            buffer[copylen] = '\0';
         }
      }
   }

   return retval;
}

int JNode_stringify_null(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_NULL, "null");
}

int JNode_stringify_true(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_TRUE, "true");
}

int JNode_stringify_false(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_FALSE, "false");
}

int JNode_stringify_string(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_STRING, (char*)node->payload);
}

void limited_long_copy(long lval, char **ptr, char *end)
{
   if (lval > 0)
   {
      int poschar = '0' + (lval % 10);
      // Recurse until we have all the digits:
      limited_long_copy(lval/10, ptr, end);

      if (*ptr < end)
      {
         **ptr = poschar;
         ++(*ptr);
      }
   }
}

int JNode_stringify_integer(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_INTEGER, (char*)node->payload);
}

int JNode_stringify_float(const JNode *node, char *buffer, int bufflen)
{
   return JNode_stringify_generic(node, buffer, bufflen, DT_FLOAT, (char*)node->payload);
}

int JNode_stringify_property(const JNode *node, char *buffer, int bufflen)
{
   return 0;
}
