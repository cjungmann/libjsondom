#ifndef JREADSTRING_H
#define JREADSTRING_H

#include <stdbool.h>

typedef struct RSHandle_s RSHandle;
typedef bool(*RSEndCheck)(char chr);

struct RSHandle_s {
   const char *string;
   char       first_char;
   char       end_signal;
   RSEndCheck end_check;
};

void ReadStringInit(RSHandle *rSHandle, char firstChar);
void ReadStringDestroy(RSHandle *rSHandle);
const char *StealReadString(RSHandle *handle);
bool JReadString(int fh, RSHandle *handle);



#endif
