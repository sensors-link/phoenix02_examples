#ifndef __STROPT_
#define __STROPT_

#include "globle.h"
#include "phnx02.h"
//typedef enum{
//    FALSE =0,
//    TRUE = !FALSE,
//}BOOL;
//
//






//extern function declare

int mystrcmp(char *str0,char *str1);
int rcv_strcmd(char * str);
int rcv_param(char str[][STR_BUFF_LEN],int len);
u32 mypow(uint8_t x,uint8_t y);
BOOL strtohex(char* str,u32 * hex);
BOOL strtodec(char* str,u32 *dec);


#endif

