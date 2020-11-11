/******************************************************************************
 * st7565r.c 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     lcd driver ic
 * Modification History
 * --------------------
 * V1.00, 31 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/
#include "st7565r.h"
#include "gpio.h"
#include "ddl.h"
#include "global.h"
#include "ctrl.h"

//#define CURRENT_CTRL                 Gpio_ReadOutputIO(GpioPortC, GpioPin13)
#define LCD_RST_HIGH           Gpio_SetIO(GpioPortA, GpioPin10)//Gpio_SetIO(GpioPortB, GpioPin15)
#define LCD_RST_LOW            Gpio_ClrIO(GpioPortA, GpioPin10)//Gpio_ClrIO(GpioPortB, GpioPin15)
#define LCD_CS_HIGH            Gpio_SetIO(GpioPortB, GpioPin12)
#define LCD_CS_LOW             Gpio_ClrIO(GpioPortB, GpioPin12)
#define LCD_RS_HIGH            Gpio_SetIO(GpioPortB, GpioPin13)
#define LCD_RS_LOW             Gpio_ClrIO(GpioPortB, GpioPin13)
#define LCD_SCLK_HIGH          Gpio_SetIO(GpioPortA, GpioPin8)
#define LCD_SCLK_LOW           Gpio_ClrIO(GpioPortA, GpioPin8)
#define LCD_SID_HIGH           Gpio_SetIO(GpioPortA, GpioPin15)
#define LCD_SID_LOW            Gpio_ClrIO(GpioPortA, GpioPin15)

//Write Command to Driver IC
void ST756x_WriteCommand(unsigned int uiCmd)   
{
	unsigned char i;
	LCD_CS_LOW;
	LCD_RS_LOW;
	for(i=0;i<8;i++)
	{
		LCD_SCLK_LOW;
		if(uiCmd&0x80) 
			LCD_SID_HIGH;
		else 
			LCD_SID_LOW;

		LCD_SCLK_HIGH;
		uiCmd = uiCmd<<1;
	}
	LCD_CS_HIGH;
}

//Write Data to Driver IC
void ST756x_WriteData(unsigned int uiData)   
{
	unsigned char i;
	LCD_CS_LOW;
	LCD_RS_HIGH;
	for(i=0;i<8;i++)
	{
		LCD_SCLK_LOW;
		if(uiData&0x80) 
			LCD_SID_HIGH;
		else 
			LCD_SID_LOW;
		LCD_SCLK_HIGH;
		uiData = uiData<<1;
	}
	LCD_CS_HIGH;
}

void LCD_Initialize(void)
{
	Delay_ms(1000);
    LCD_RST_LOW;                /*低电平复位*/
    Delay_ms(20);
    LCD_RST_HIGH;		        /*复位完毕*/
    Delay_ms(20);        
//	ST756x_WriteCommand(0xE2);	/*软复位*/
//	Delay_ms(5);
	ST756x_WriteCommand(0x2C);  /*升压步聚1*/
	Delay_ms(5);	
	ST756x_WriteCommand(0x2E);  /*升压步聚2*/
	Delay_ms(5);
	ST756x_WriteCommand(0x2F);  /*升压步聚3*/
	Delay_ms(5);
	ST756x_WriteCommand(0x24);  /*粗调对比度，可设置范围0x20～0x27*/
	ST756x_WriteCommand(0x81);  /*微调对比度*/
	ST756x_WriteCommand(0x2F);  /*0x28,微调对比度的值，可设置范围0x00～0x3f*/
	ST756x_WriteCommand(0xA2);  /*1/9偏压比（bias）*/
	ST756x_WriteCommand(0xC8);  /*行扫描顺序：从上到下*/
	ST756x_WriteCommand(0xA0);  /*列扫描顺序：从左到右*/
	ST756x_WriteCommand(0x40);  /*起始行：第一行开始*/
	ST756x_WriteCommand(0xAF);  /*开显示*/

}

void LCD_ClearScreen(void)
{
    unsigned char i,j;
	for(i=0;i<8;i++)
	{
		ST756x_WriteCommand(0xB0+i);
		ST756x_WriteCommand(0x10);
		ST756x_WriteCommand(0x00);
		for(j=0;j<128;j++)
		{
		  	ST756x_WriteData(0x00);
		}
	}
}

void LCD_DisplayControl(unsigned char ucOnOff)
{
	if(ucOnOff)
		ST756x_WriteCommand(0xAF);
	else
		ST756x_WriteCommand(0xAE);
}

void LCD_Display5x7Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData)
{
	unsigned char ucCol;
	unsigned int uiPageAddress;
	unsigned char ucCol_L,ucCol_H;
	
	uiPageAddress = 0xB0+ucPage-1;
	
	ucCol_L =(ucColumn&0x0F)-1;
	ucCol_H =((ucColumn>>4)&0x0F)+0x10;
	
	ST756x_WriteCommand(uiPageAddress); /*Set Page Address*/
	ST756x_WriteCommand(ucCol_H);	    /*Set MSB of column Address*/
	ST756x_WriteCommand(ucCol_L);	    /*Set LSB of column Address*/
	
	for (ucCol=0;ucCol<6;ucCol++)
	{	
		ST756x_WriteData(*pData);
		pData++;
	}

}

void LCD_Display8x16Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData)
{
	unsigned char ucCol;
	unsigned int uiPageAddress;
	unsigned char ucCol_L,ucCol_H;
	unsigned char i;
	uiPageAddress = 0xB0+ucPage-1;
	
	ucCol_L =(ucColumn&0x0F)-1;
	ucCol_H =((ucColumn>>4)&0x0F)+0x10;
		
	for(i=0;i<2;i++)
	{
		ST756x_WriteCommand(uiPageAddress); /*Set Page Address*/
		ST756x_WriteCommand(ucCol_H);		/*Set MSB of column Address*/
		ST756x_WriteCommand(ucCol_L);		/*Set LSB of column Address*/

		for (ucCol=0;ucCol<8;ucCol++)
		{	
			ST756x_WriteData(*pData);
			pData++;
		}
		uiPageAddress++;
	}

}

void LCD_Display16x16Graphic(unsigned char ucPage,unsigned char ucColumn,unsigned char *pData)
{
	unsigned char ucCol;
	unsigned int uiPageAddress;
	unsigned char ucCol_L,ucCol_H;
	unsigned char i;
	uiPageAddress = 0xB0+ucPage-1;
	
	ucCol_L =(ucColumn&0x0F)-1;
	ucCol_H =((ucColumn>>4)&0x0F)+0x10;
		
	for(i=0;i<2;i++)
	{
		ST756x_WriteCommand(uiPageAddress); /*Set Page Address*/
		ST756x_WriteCommand(ucCol_H);		/*Set MSB of column Address*/
		ST756x_WriteCommand(ucCol_L);		/*Set LSB of column Address*/

		for (ucCol=0;ucCol<16;ucCol++)
		{	
			ST756x_WriteData(*pData);
			pData++;
		}
		uiPageAddress++;
	}

}

void LCD_Display128x64Graphic(unsigned char *pData)
{
	unsigned char ucCol;
	unsigned int uiPageAddress;
	unsigned char ucCol_L,ucCol_H;
	unsigned char i;
	
	uiPageAddress = 0xB0;	
	ucCol_L = 0x00;
	ucCol_H = 0x10;
		
	for(i=0;i<8;i++)
	{
		ST756x_WriteCommand(uiPageAddress); /*Set Page Address*/
		ST756x_WriteCommand(ucCol_H);		/*Set MSB of column Address*/
		ST756x_WriteCommand(ucCol_L);		/*Set LSB of column Address*/

		for (ucCol=0;ucCol<128;ucCol++)
		{	
			ST756x_WriteData(*pData);
			pData++;
		}
		uiPageAddress++;
	}

}

void LCD_DisplayLattice(unsigned char ucData1,unsigned char ucData2)
{  
    unsigned char ucSeg;
    unsigned char ucPage;
    for(ucPage=0xB0;ucPage<0xB8;ucPage++)
    {
        ST756x_WriteCommand(ucPage);
        ST756x_WriteCommand(0x10);
        ST756x_WriteCommand(0x00);
        for(ucSeg=0;ucSeg<64;ucSeg++)
        { 
            ST756x_WriteData(ucData1);
            ST756x_WriteData(ucData2);
        }
    }
}   

