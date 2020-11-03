/**
 * @file e2prom.c
 * @author 唐碧飞
 * @brief E2PROM操作函数
 * 使用系统资源：
 *   无
 * 使用全局变量：
 *   无
 * 依赖应用模块：
 *   efc
 *   globel
 * @version 0.1
 * @date 2020-07-09
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "e2prom.h"
#include "lib_include.h"
#include "globle.h"





/**
 * @brief 写数据到E2PROM
 *
 * @param addr 地址
 * @param dat 数据
 */
void WriteDataToE2PROM(u16 addr,u16 dat)
{
	u32 u32addr = E2PROM_BASE_ADDR + addr;
    u32 u32Dat = ( (u32)~dat<<16) | dat;
    if( EFC_EEPROMWrite(u32addr,u32Dat,EFC_PRG_WORD) != TRUE )
    {
        //写失败重试多次
		int i;
		for(i=0;i<WR_E2PROM_FAIL_RETRY_TIMES;++i)
		{
			if( (EFC_EEPROMWrite(u32addr,u32Dat,EFC_PRG_WORD)) == TRUE )
				break;
		}
		if( i == WR_E2PROM_FAIL_RETRY_TIMES )
		{
			SoftReset();
		}
    }
}
/**
 * @brief 从E2PROM里读数据
 *
 * @param addr 地址
 * @param dat 数据
 * @return BOOL TRUE 成功，FLALS 失败
 */
BOOL ReadDataFromE2PROM(u16 addr,u16 *dat)
{
    int i;
	u32 u32addr = E2PROM_BASE_ADDR + addr;
	for(i=0;i<WR_E2PROM_FAIL_RETRY_TIMES;++i)
    {
        u32 u32Dat = REG32(u32addr);
        u16 tmph,tmpl;
        tmph = ~(u32Dat>>16);
        tmpl = u32Dat&0xffff;
        if( tmph == tmpl )
        {
            *dat = tmpl;
            return TRUE;
        }
    }
    return FALSE;
}







