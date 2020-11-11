/******************************************************************************
 * st7565r.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     lcd driver ic inlcude file
 * Modification History
 * --------------------
 * V1.00, 31 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/
#ifndef _ST7565R_H_
#define _ST7565R_H_    1

extern void LCD_Initialize(void);
extern void LCD_ClearScreen(void);
extern void LCD_DisplayControl(unsigned char ucOnOff);
extern void LCD_Display5x7Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData);
extern void LCD_Display8x16Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData);
extern void LCD_Display16x16Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData);
extern void LCD_Display128x64Graphic(unsigned char *pData);
extern void LCD_DisplayLattice(unsigned char ucData1,unsigned char ucData2);

#endif
