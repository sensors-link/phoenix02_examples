



#include "lib_include.h"


#define LED_PIN   GPIO_PIN8

#define LED_ON     GPIO_SetPin(LED_PIN)
#define LED_OFF    GPIO_ClrPin(LED_PIN)



#define   EEPROM_ADDR  0x10180000
#define   EEPROM_DATA  0x55aa55aa

int main(void)
{
	debug_frmwrk_init();
	_DBG_("test");
    GPIO_PinConfigure(LED_PIN,DISABLE,ENABLE,ENABLE,DISABLE,DISABLE);
    EFC_Init();

    if(EFC_EEPROMWrite(EEPROM_ADDR,EEPROM_DATA,EFC_PRG_WORD)==TRUE)
    {
        if(REG32(EEPROM_ADDR)==EEPROM_DATA)
        {
            LED_ON;
        	_DBG_("success");
        }
        else
        {
            LED_OFF;
            _DBG_("fail0");
        }

    }
    else
    {
        LED_OFF;
        _DBG_("fail1");
    }
    while(1);
}



