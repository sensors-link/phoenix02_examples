/*
 ============================================================================
 Name        : main.c
 Author      : tbf
 Version     :
 Copyright   : Your copyright notice
 Description : Hello RISC-V World in C
 ============================================================================
 */


#include <stdio.h>
// #define _DEBUG
#include "lib_include.h"

int main(void)
{
	printf("Hello world");
	char a[] = {'a', '\0'};
	while(1)
		{
		printf(a);
		a[0] ++;
		if ( a[0] > '9') a[0] = '0';
		}
	return 0;
}

void _fini(){}