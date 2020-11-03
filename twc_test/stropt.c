#include "globle.h"
#include "stropt.h"
#include "uart.h"




int mystrcmp(char *str0,char *str1)
{
    while(*str1 != '\0')
    {
	if( *str0++ != *str1++)
	{
	    return 0;
	}
    }
    return 1;
}

int rcv_strcmd(char * str)   //0d 0a end
{
    int pos=0;
    while(1)
    {
        *str = UART_Receive(UART1);
        if(*str == ' ')
        {
            break;
        }
        else
        {
        ++pos;
        if( pos > STR_BUFF_LEN-1)
            pos = STR_BUFF_LEN-1;
        else
            ++str;
        }
    }
    *str = 0;
    return pos;
}


int rcv_param(char str[][STR_BUFF_LEN],int len)  //space end
{
    int i=0;
    u8 pos=0;
    int end_ready,end_ok=0;
    char *pstr;
	while(1)
	{
        pstr = str[i];
        while(1)
        {
            *pstr = UART_Receive(UART1);
            if(*pstr == ' ')
            {
                break;
            }
            if(*pstr == '\r')
            {
                end_ready=1;
            }
            else
            {
                if(*pstr == '\n')
                {
                    if(end_ready)
                    {
                        end_ok=1;
                    }
                    else
                    {
                        end_ready=0;
                    }
                }
                else
                {
                ++pos;
                if( pos > STR_BUFF_LEN-1)
                    pos = STR_BUFF_LEN-1;
                else
                    pstr++;
                }
            }
            if(end_ok)
            {
                *pstr = 0;
                return pos;
            }
        }
        *pstr = 0;
        ++i;
        if(i>len)
            i=len;
    }
}

u32 mypow(u8 x,u8 y)
{
    u32 rst=1;
    for(;y>0;--y)
        rst *= x;
    return rst;
}

BOOL strtohex(char* str,u32 *hex)
{
    int i;int len;
    *hex=0;
    char *tmp=str;
    for(i=0;*tmp!='\0';++i)
        ++tmp;
    len = i;
    for(;i>0;--i)
    {
	if( (str[i-1]>='0') && (str[i-1]<='9') )
	    *hex += (str[i-1]-'0')*mypow(16,len-i);
	else if( (str[i-1]>='a') && (str[i-1]<='f') )
	    *hex += ((str[i-1]-'a')+10)*mypow(16,len-i);
	else if( (str[i-1]>='A') && (str[i-1]<='F') )
	    *hex += ((str[i-1]-'A')+10)*mypow(16,len-i);
	else
	    return FALSE;
    }

    return TRUE;
}
BOOL strtodec(char* str,u32 *dec)
{
    dec = 0;
    while(*str)
    {
	int i;int len;
	*dec=0;
	char *tmp=str;
	dec = 0;
	for(i=0;*tmp!='\0';++i)
	    ++tmp;
	len = i;
	for(;i>0;--i)
	{
	    if( (str[i-1]>='0') && (str[i-1]<='9') )
		*dec += (str[i-1]-'0')*mypow(10,len-i);
	    else
		return FALSE;
	}
    }
    return TRUE;
}
