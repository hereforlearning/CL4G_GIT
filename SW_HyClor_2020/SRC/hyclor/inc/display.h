/******************************************************************************
 * display.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     display inlcude file
 * Modification History
 * --------------------
 * V1.00, 1 Apr 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_    1

extern void LCDInitialize(void);
extern void LCDDisplayClear(void);
extern void LCDDisplayControl(unsigned char ucOnOff);
extern void LCDInspection(void);

#endif
