#include "jsondom.h"
#include "JParser.h"  // for Report_Error function pointer
#include <string.h>   // for strlen, memcpy, etc
#include <stdlib.h>   // for labs

jd_NodeError jn_error = JNE_SUCCESS;

int jd_Node_stringify_generic(const jd_Node *node,
                            char *buffer,
                            int bufflen,
                            jd_Type type,
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

int jd_Node_stringify_null(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_NULL, "null");
}

int jd_Node_stringify_true(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_TRUE, "true");
}

int jd_Node_stringify_false(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_FALSE, "false");
}

int jd_Node_stringify_string(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_STRING, (char*)node->payload);
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

int jd_Node_stringify_integer(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_INTEGER, (char*)node->payload);
}

int jd_Node_stringify_float(const jd_Node *node, char *buffer, int bufflen)
{
   return jd_Node_stringify_generic(node, buffer, bufflen, JD_FLOAT, (char*)node->payload);
}

int jd_Node_stringify_property(const jd_Node *node, char *buffer, int bufflen)
{
   return 0;
}
