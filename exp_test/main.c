/*
 ============================================================================
 Name        : main.c
 Author      : tbf
 Version     :
 Copyright   : Your copyright notice
 Description : Hello RISC-V World in C
 ============================================================================
 */


#include "lib_include.h"


int main(void)
{
    int i;
	debug_frmwrk_init();
	_DBG_("test");
    while(1)
    {
        u8 cmd[20];
        u8 chTmp = _DG;
        if( chTmp == 0xcc)
        {
            u8 len = _DG;
            for(i=0;i<len;++i)
            {
                cmd[i] = _DG;
            }
            for(i=0;i<len;++i)
            {
                _DBC(cmd[i]);_DBC(' ');
            }
            if( cmd[0] == 0) //soft int
            {
                if(cmd[1] == 0)
                    EnableGlobleIRQ();
                else
                    DisableGlobleIRQ();
                if( cmd[2] == 0)
                    EnableSoftIRQ();
                else
                    DisableSoftIRQ();
                SoftTrigIRQ();
            }

            if( cmd[0] == 1)  //pmu
            {
                PMU_Init();
                if( cmd[1] == 0)
                    PMU_SoftChipReset();
                else if( cmd[1] == 1)
                    PMU_SoftChipReset();
                else if( cmd[1] == 2)
                    PMU_SoftCoreReset();
                else if( cmd[1] == 3)
                {
                    WDT_Init(1000,PMU_CR_LPTCLKSEL_LRC,WDT_OV_INT);
                    PMU_EnterSleep();
                    _DBG_("sleep wake");
                }
                else if( cmd[1] == 4)
                {
                    WDT_Init(1000,PMU_CR_LPTCLKSEL_LRC,WDT_OV_INT);
                    PMU_EnterDeepSleep();
                    _DBG_("deep wake");
                }
                else if( cmd[1] == 5)
                {
                    WDT_Init(1000,PMU_CR_LPTCLKSEL_LRC,WDT_OV_INT);
                    // PMU_EnterPowerDown();//fpga no
                    _DBG_("pd wake");  //nomarl not ext
                }
            }
            if( cmd[0] == 2)         //plic test
            {
                LPT_Init(PMU_CR_LPTCLKSEL_LRC,1000,LPT_PIT_CNT);
                PLIC_SetThreshold(0);
                PLIC_SetPriority(LPT_IRQn,1);
                if( cmd[1] == 0)
                    PLIC_EnableIRQ(LPT_IRQn);
                else
                    PLIC_DisableIRQ(LPT_IRQn);
                LPT_EnableIRQ();
                EnableExtIRQ();
            }
            if( cmd[0] == 3)
            {
                int tmp = LPT_GetCount();
                _DBD32(tmp);
            }
            if( cmd[0] == 4)         //plic test
            {
                DisableExtIRQ();
            	TIM_TimerInit(TIM1,TIM_TM_AUTO_RUN,1000);
                TIM_TimerInit(TIM2,TIM_TM_AUTO_RUN,1000);
                PLIC_SetThreshold(0);
                PLIC_SetPriority(TIMER1_IRQn,cmd[1]);
                PLIC_SetPriority(TIMER2_IRQn,cmd[2]);
                PLIC_EnableIRQ(TIMER1_IRQn);
                PLIC_EnableIRQ(TIMER2_IRQn);
                TIM_EnableIRQ(TIM1);
                TIM_EnableIRQ(TIM2);
                for(i=80000;i>0;--i);
                EnableExtIRQ();
            }



        }

    }
	return 0;
}
