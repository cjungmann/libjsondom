#include <stdbool.h>
#include <ctype.h>    // for isdigit()

bool isJsonNumber(const char *str, bool *isFloatReturn)
{
   bool retval = false;

   bool is_float = false;
   bool has_numerals = false;
   bool has_decimal = false;
   bool is_exponent = false;

   const char *ptr = str;
   int first_numeral = '\0';

   // Leading sign is permitted and skipped if found:
   if (*ptr == '-' || *ptr == '+')
      ++ptr;

   while (*ptr)
   {
      // RFC 8259 doesn't allow hex numbers, and we also
      // won't try to parse octal or binary prefixes
      if (isdigit(*ptr))
      {
         has_numerals = true;
         // Save first numeral for later check for forbidden initial '0'
         if (first_numeral ==  '\0')
            first_numeral = *ptr;
      }
      else if (*ptr == '.')
      {
         // .123 and -.123 are invalid
         if ( ptr==str || !isdigit(*(ptr-1)))
            goto early_exit;
         // encountering a second period or a period
         // in the exponent are invalid:
         else if (has_decimal || is_exponent)
            goto early_exit;
         else
            has_decimal = is_float = true;
      }
      else if (*ptr=='e' || *ptr=='E')
      {
         // [Ee] after exponent is an error:
         if (is_exponent)
            goto early_exit;

         // An exponent must have a coefficient (number before the 'e')
         if (!has_numerals)
            goto early_exit;

         // Next char can be a sign:
         if (*(ptr+1) == '-' || *(ptr+1) == '+')
            ++ptr;

         is_exponent = is_float= true;

         // Clear flag to require numerals after the E:
         has_numerals = false;
      }
      else
      {
         // Any unexpected characters rule-out proper number:
         goto early_exit;
      }

      ++ptr;
   }

   // It's only a number if several factors line up:
   retval = has_numerals && (first_numeral!='0' || is_exponent || has_decimal);

  early_exit:
   if (isFloatReturn)
   {
      if (retval && is_float)
         *isFloatReturn = true;
      else
         *isFloatReturn = false;
   }

   return retval;
}

