/******************************************************************************
 * keypad.c 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     key process 
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/
#include "gpio.h"
#include "keypad.h"
#include "global.h"
#include "ctrl.h"

unsigned char g_bKeyDetect;
unsigned char g_bKeyRepeat;
unsigned char g_bKey1stRepeat;
unsigned char g_bKeyCheckRepeat;
unsigned char g_bFastRepeat;

unsigned char g_ucKeyCounter;
unsigned char g_ucKeyDebounce;
KeyScanType   g_LastKeyStatus;
KeyScanType   g_KeyCode;


#define SWITCH_UP    Gpio_GetInputIO(GpioPortA, GpioPin5)
#define SWITCH_DOWN  Gpio_GetInputIO(GpioPortA, GpioPin4)

 
void KeyVariableInitial(void)
{
 g_ucKeyCounter = KEY_DEBOUNCE_TIME;
 g_LastKeyStatus = KEY_NOTHING;
 g_KeyCode = KEY_NOTHING;
 g_bKeyDetect = 0;
 g_bKeyRepeat = 0;
 g_bKey1stRepeat = 1;
 g_bKeyCheckRepeat = 0;
 g_ucKeyDebounce = 0;
 g_bFastRepeat = 0;
}

unsigned char ReadKey(void)
{
	unsigned char ucKey=0x00;
	
	if(SWITCH_UP == _LOW)
	{
		ucKey |= KEYCODE_UP;
	}
	if(SWITCH_DOWN == _LOW)
	{
		ucKey |= KEYCODE_DOWN;
	}

	return ucKey;
}

void KeyDetectTimer(void)
{
 if (g_ucKeyCounter)
	 g_ucKeyCounter--; // descrease counter
}

KeyScanType KeyScanStatus(void)
{
	unsigned char ucTemp;
	KeyScanType KeyStatus = KEY_NOTHING; // key status
	ucTemp=ReadKey();

	switch(ucTemp)
	{
		case KEYCODE_UP:
			KeyStatus |= KEY_UP;
			break;
		case KEYCODE_DOWN:
			KeyStatus |= KEY_DOWN;
			break;
		case KEYCODE_UP_DOWN:
			KeyStatus |= KEY_UP_DOWN;
			break;
	}


	return KeyStatus;
}

void KeyDetect(void)
{
 KeyScanType ucStatusBff; // status buffer

 if (g_ucKeyCounter == 0) // check counter
 {
	 ucStatusBff = KeyScanStatus(); // scan key status
	 
	 if (ucStatusBff == g_LastKeyStatus) // match last
	 {
		 if (g_bKeyDetect) // check key busy
			 ;
		 else if (g_LastKeyStatus) // check key pressed
		 {
			 if (g_KeyCode == g_LastKeyStatus) // check repeat
			 {
				 if (g_bKeyCheckRepeat) // need to repeat
				 {
					 if (g_bKey1stRepeat)
					 {
						 g_ucKeyDebounce++;
						 if (g_ucKeyDebounce==KEY_DEBOUNCE_REPEAT)
						 {
							 g_ucKeyDebounce = 0;
							 g_bKey1stRepeat = 0;
							 g_bKeyDetect = 1; // detect key
						 }
					 }
					 else
					 {
						 g_bKeyRepeat = 1;
						 g_ucKeyDebounce++;
						 if ((!g_bFastRepeat && g_ucKeyDebounce==KEY_DEBOUNCE_MIDDLE)
						   || (g_bFastRepeat && g_ucKeyDebounce==KEY_DEBOUNCE_FAST))
						 {
							 g_ucKeyDebounce = 0;
							 g_bKeyDetect = 1; // detect key
						 }
					 }
				 }
			 }
			 else // new key pressed
			 {
				 g_KeyCode = g_LastKeyStatus;
				 g_bKeyDetect = 1; // detect key
				 g_bKeyRepeat = 0;
				 g_bKey1stRepeat = 1;
				 g_bKeyCheckRepeat = 1;
				 g_ucKeyDebounce = 0;
				 g_bFastRepeat = 0;
			 }
		 }
		 else // no key pressed
		 {
			 g_KeyCode = KEY_NOTHING; // reset key code
		 }
	 }
	 else // different
	 {
		 g_LastKeyStatus = ucStatusBff; // refresh key status
		 g_bKeyRepeat = 0;
		 g_bKey1stRepeat = 1;
		 g_bKeyCheckRepeat = 0;
		 g_ucKeyDebounce = 0;
		 g_bFastRepeat = 0;
	 }
	 g_ucKeyCounter = KEY_DEBOUNCE_TIME; // next time counter
 }
}

void KeyHandler(void)
{
	 KeyDetect();

	 if(g_bKeyDetect)
	 {
		 switch (g_KeyCode)
		 {
			 case KEY_UP: 
			 	CellOutputCurrentAdjust(1);
			 	break; 				 
			 case KEY_DOWN: 		 	
			 	CellOutputCurrentAdjust(2);
			 	break; 				 
			 case KEY_UP_DOWN:
			 	if(g_bKeyRepeat)
			 	{
					g_bKeyRepeat = 0;
					break;
			 	}
				ComboKeyProcess();
			 	break;
			 default: 
			 	break;
		 }
		 SpecialSecretCodeProcess(g_KeyCode);
		 g_bKeyDetect = 0; // release to detect key
	 }

}
