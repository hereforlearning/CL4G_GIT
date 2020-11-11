/******************************************************************************
 * main.c 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     main file
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/
#include "global.h"
#include "mcu.h"
#include "keypad.h"
#include "ctrl.h"
#if(BOARD_SELECT == BOARD_LCD_ENCODER)
#include "display.h"
#endif
int main(void)
{

	MCU_Initialize();             //Initial all MCU settings
	#if(BOARD_SELECT == BOARD_LCD_ENCODER)
	LCDInitialize();
	#endif
	PowerOnInitialize();          //Initial all outputs
	#if(BOARD_SELECT == BOARD_LCD_ENCODER)
	LCDInspection();
	#endif
	while(1)
	{
		KeyHandler();		      //Key Process
		ProcessHandler();		  //Event Process
	}

}
