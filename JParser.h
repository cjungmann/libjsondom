#ifndef JPARSER_H
#define JPARSER_H

#include "JNode.h"
#include "jsondom.h"

void report_parse_error(jd_ParseError *pe, int fh, const char *message);

/**
 * Function type for overriding standard error reporter
 *
 * @return True to exit, false to continue
 * */
typedef bool (*Error_Reporter)(
   int source_fh,       /**<
                         * @brief handle to JSON document source
                         * @details
                         *    Can be used to determine the location
                         *    of the error in the source file
                         */
   const char *format,  ///< printf-style format string
   ...                  ///< arguments to fill conversion specification
                        ///< of the format string
   );

/** Default implementation of Error_Reporter */
bool Standard_Report_Error(int source_fh, const char *format, ...);
extern Error_Reporter Report_Error;


/** typedef of CollectionTools_s */
typedef struct CollectionTools_s CollectionTools;

/** typedef member of CollectionTools */
typedef bool (*Is_End_Char)(char chr);
/** typedef member of CollectionTools */
typedef bool (*Coerce_Type)(JNode *node);
/** typedef member of CollectionTools */
typedef bool (*Read_Member)(int fh,
                            JNode *parent,
                            JNode **new_node,
                            char first_char,
                            char *signal_char,
                            jd_ParseError *pe
   );

/**
 * @brief Small collection of tools for parsing JSON data
 * @details
 *    The function pointers in the struct are used by JParser
 *    to perform context-dependent parsing.  Once the context is
 *    determined, the appropriate CollectionTools instance will be
 *    used to test for completion (Is_End_Char), transform the
 *    in-process JNode to the appropriate type (Coerce_Type), or to
 *    to read properties for objects or elements for arrays
 *    (Read_Member).  This is somewhat more efficient but also
 *    makes the code easier to understand.
 */
struct CollectionTools_s {
   Is_End_Char    is_end_char; /**<
                                * @brief
                                *    Pointer to function that tests if
                                *    a given character signals the end
                                *    of the current collection
                                */

   Coerce_Type    coerce_type; /**<
                                * @brief
                                *    Pointer to function that will be
                                *    used to convert a generic JNode
                                *    into the specific JNode
                                *    collection type required
                                */
   Read_Member    read_member; /**<
                                * @brief
                                *    Pointer to function that will
                                *    create the appropriate members
                                *    for the current collection type
                                */
   char           end_char;    /**<
                                * @brief
                                *    Make this character available
                                *    for error reporting
                                */
};


/**
 * @ingroup AllFunctions
 */
bool JParser(int           fh,
             JNode         *parent,
             JNode         **node,
             char          first_char,
             char          *signal_char,
             jd_ParseError *parse_error
   );

bool confirm_no_further_file_content(int fh);


#endif
