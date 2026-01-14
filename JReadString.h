/** @file JReadString.h */

#ifndef JREADSTRING_H
#define JREADSTRING_H

#include <stdbool.h>
#include "jsondom.h"

/** Typedef of RSHandle_s struct */
typedef struct RSHandle_s RSHandle;

/** Function typedef for member of #RSHandle_s struct */
typedef bool(*RSEndCheck)(char chr);

/**
 * @brief Working values for running ReadString functions
 */
struct RSHandle_s {
   const char *string;     ///< Address at which the complete string will be found
   char       first_char;  /**< @brief Character that begins the string
                            *
                            *  @details
                            *     If this is a double-quote, the string will end
                            *     with the next unescaped double-quote character.
                            *     If this is __not__ a double-quote, the first
                            *     unescaped whitespace character will terminate the
                            *     string value.
                            */
   char       end_signal;  /**< @brief Saves the character that terminated the string.
                            *
                            * @details
                            *    For strings not terminated by a double-quote, this
                            *    value may be needed for the next string, saved because
                            *    it will have already been read from the text source.
                            */
   RSEndCheck end_check;   /**< @brief Pointer to function that tests for end-of-string.
                            *
                            * @details
                            *    Conditionally set the test function at the beginning
                            *    of processing (based in @ref first_char) rather than
                            *    with a conditional test with each character read.
                            */
};

/**
 * @ingroup AllFunctions
 * @defgroup ReadStringFuncs Functions manage streaming string data
 * @{
 */
void ReadStringInit(RSHandle *rSHandle, char firstChar);
void ReadStringDestroy(RSHandle *rSHandle);
const char *StealReadString(RSHandle *handle);
bool JReadString(int fh, RSHandle *handle, jd_ParseError *pe);
/** @} */



#endif
