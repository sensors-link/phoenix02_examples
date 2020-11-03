

#ifdef _DEBUG
#include "phnx02.h"
#include "lib_include.h"

typedef union RcvSendBuff
{
    unsigned char ucData[4];
    struct addr
    {
        unsigned long ulAddr:31;
        unsigned long ulRD_WR:1;
    }__attribute__((packed)) sAddr;
    unsigned long ulData;
}RcvSendBuff_t;

static RcvSendBuff_t  gsRcvSendBuff;
static unsigned long gRcvAddr;
unsigned long gRcvEnalbe;
static unsigned long gRcvPos,gSendPos;
static unsigned long gRcvAddrFlag;

void UART1_IrqHandler(void){
#ifdef _WDT_EN
    WDT_ClrCount();
#endif
    if( UART1->ISR & UART_ISR_RI )  //接收中断
    {
        UART1->ISR = UART_ISR_RI;
        if((!gRcvEnalbe) && (UART1->SBUF == 0xcc) )
        {
            gRcvEnalbe = 1;
            gRcvPos = 0;
            gRcvAddrFlag = 1; //开始接收地址
        }
        else
        {
            gsRcvSendBuff.ucData[gRcvPos] = UART1->SBUF;
            ++gRcvPos;
            if( gRcvPos >= 4)
            {
                if( gRcvAddrFlag )  //接收地址
                {
                    if( gsRcvSendBuff.sAddr.ulRD_WR )  //若为读命令停止接收准备发送数据
                    {
                        gRcvEnalbe = 0;
                        gsRcvSendBuff.ulData = REG32(gsRcvSendBuff.sAddr.ulAddr);//读地址数据
                        //printf("rddat:%x\r\n",gsRcvSendBuff.ulData);
                        gSendPos = 0;
                        UART1->ISR = UART_ISR_TI;
                        UART_EnableIRQ(UART1,UART_IRQ_TYPE_TX);//使能发送中断
                        UART1->SBUF = gsRcvSendBuff.ucData[gSendPos];
                    }
                    else            //写命令
                    {
                        gRcvAddr = gsRcvSendBuff.sAddr.ulAddr;   //保存接收地址
                        gRcvAddrFlag = 0;
                        gRcvPos = 0;  //重新准备接收数据
                    }
                }
                else   //接收数据
                {
                    REG32(gRcvAddr) = gsRcvSendBuff.ulData;
                    //printf("wraddr%x,data:%x\r\n",gRcvAddr,gsRcvSendBuff.ulData);
                    gRcvEnalbe = 0;
                }
            }
        }
    }
    else                  //发送中断
    {
        UART1->ISR = UART_ISR_TI;
        ++gSendPos;
        if( gSendPos >=4 )
        {
            printf("send end\r\n");
            UART_DisableIRQ(UART1,UART_ISR_TI);
        }
        else
        {
        	//printf("no:%d,dat:%c\r\n",gSendPos,gsRcvSendBuff.ucData[gSendPos]);
        	UART1->SBUF = gsRcvSendBuff.ucData[gSendPos];
        }
    }
};





#endif //_DEBUG
