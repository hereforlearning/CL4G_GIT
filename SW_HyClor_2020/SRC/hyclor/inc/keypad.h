/******************************************************************************
 * keypad.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     keypad include file
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#ifndef _KEYPAD_H_
		  
#define _KEYPAD_H_
		  
#define KEY_DEBOUNCE_REPEAT     20 // KEY_DEBOUNCE_TIME*KEY_DEBOUNCE_REPEAT(ms)
#define KEY_DEBOUNCE_MIDDLE     6  // KEY_DEBOUNCE_TIME*KEY_DEBOUNCE_MIDDLE(ms)
#define KEY_DEBOUNCE_FAST       3  // KEY_DEBOUNCE_TIME*KEY_DEBOUNCE_FAST(ms)
#define KEY_DEBOUNCE_TIME       20 // unit: 1ms

#define KEYCODE_UP				0x01
#define KEYCODE_DOWN			0x02
#define KEYCODE_UP_DOWN			0x03  
// key scan
typedef enum _KeyScanType
{
	KEY_NOTHING		  =0x00,
	KEY_UP			  ,
	KEY_DOWN			  ,
	KEY_UP_DOWN	  
} KeyScanType;

//////////////////////////////////////////////////////////////
// Variable
//////////////////////////////////////////////////////////////


extern unsigned char g_bKeyDetect;
extern KeyScanType   g_KeyCode;

extern void KeyVariableInitial(void);
extern void KeyDetectTimer(void);
extern KeyScanType KeyScanStatus(void);
extern void KeyDetect(void);
extern void KeyHandler(void);

#endif /* _KEYPAD_H_ */

