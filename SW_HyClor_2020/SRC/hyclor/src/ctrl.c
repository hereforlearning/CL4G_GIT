/******************************************************************************
 * ctrl.c 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     all control function
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#include "gpio.h"
#include "flash.h"
#include "ddl.h"
#include "global.h"
#include "keypad.h"
#include "mcu.h"
#include "app_uart.h"

#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
//Control IO definition
#define CURRENT_CTRL                 Gpio_ReadOutputIO(GpioPortC, GpioPin13)
#define CURRENT_CTRL_OFF              Gpio_SetIO(GpioPortC, GpioPin13)
#define CURRENT_CTRL_ON             Gpio_ClrIO(GpioPortC, GpioPin13)

#define CELL_POLARITY_HIGH	         Gpio_SetIO(GpioPortD, GpioPin1)
#define CELL_POLARITY_LOW	         Gpio_ClrIO(GpioPortD, GpioPin1)

#define CELL_POLARITY2_HIGH	         Gpio_SetIO(GpioPortD, GpioPin0)
#define CELL_POLARITY2_LOW           Gpio_ClrIO(GpioPortD, GpioPin0)

#else
//Control IO definition
#define CURRENT_CTRL                 Gpio_ReadOutputIO(GpioPortC, GpioPin13)
#define CURRENT_CTRL_ON            Gpio_SetIO(GpioPortC, GpioPin13)
#define CURRENT_CTRL_OFF             Gpio_ClrIO(GpioPortC, GpioPin13)

#define CELL_POLARITY_HIGH	         Gpio_SetIO(GpioPortD, GpioPin0)
#define CELL_POLARITY_LOW	         Gpio_ClrIO(GpioPortD, GpioPin0)

//#define CELL_POLARITY2_HIGH	         Gpio_SetIO(GpioPortD, GpioPin1)
//#define CELL_POLARITY2_LOW           Gpio_ClrIO(GpioPortD, GpioPin1)
#endif
//#define FAN_CTRL_HIGH
#if(WATER_FLOW_CHECK == _ENABLE)
#if(WATER_SENSOR_TYPE == EXT_SENSOR)
#define WATER_FLOW_SENSOR            Gpio_GetInputIO(GpioPortA, GpioPin0)
#else
#define WATER_FLOW_SENSOR            Gpio_GetInputIO(GpioPortA, GpioPin1)
#endif
#endif
//EEPROM Address definition
#define EEPROM_START_ADDRESS         0

#define EEPROM_CHECK1_ADDRESS        EEPROM_START_ADDRESS			 //0x00
#define EEPROM_CHECK2_ADDRESS        EEPROM_CHECK1_ADDRESS + 1       //0x01

#define MODEL_NUMBER_ADDR			 EEPROM_CHECK2_ADDRESS + 1       //0x02
#define CELL_POLARITY_ADDR    		 MODEL_NUMBER_ADDR + 1           //0x03
#define CELL_CURRENT_ADDR            CELL_POLARITY_ADDR + 1          //0x04
#define CELL_REVERSE_SETTING_ADDR    CELL_CURRENT_ADDR + 1           //0x05
#define CELL_REVERSE_TIME_ADDR	     CELL_REVERSE_SETTING_ADDR + 1   //0x06
#define PREMANENT_BOOST_ADDR         CELL_REVERSE_TIME_ADDR + 1      //0x07
#define TEST_MODE_FLAG_ADDR          PREMANENT_BOOST_ADDR + 1        //0x08
#define CURRENT_CAL_ADDR             TEST_MODE_FLAG_ADDR + 1         //0x09
#define R_CURRENT_CAL_ADDR           CURRENT_CAL_ADDR + 1            //0x0A

#if(RUNNING_TIME_DISPLAY == _ENABLE)
#define CELL_RUNNING_MINS_ADDR       0x201 
#define CELL_RUNNING_TIME_ADDR       CELL_RUNNING_MINS_ADDR + 1
#endif

#define DATA_BACKUP_ADDR             0x400

#define ADC_STABLE_NUM               100//50
#define ADC_STABLE_NUM2              (ADC_STABLE_NUM - 2)

enum
{
	_NOTHING = 0,
	_USER_SETTING = BIT0,	
	_RUNNING_DATA = BIT1,
	_FACTORY_DATA = BIT2
};

enum
{
	LED_1 =BIT0,
	LED_2 =BIT1,
	LED_3 =BIT2,
	LED_4 =BIT3,
	LED_5 =BIT4,
	LED_6 =BIT5,
	LED_7 =BIT6,
	LED_8 =BIT7,
	LED_9 =BIT8,
	LED_10 =BIT9,
	LED_11 =BIT10,
	LED_12 =BIT11
};
#if(BRAND_SELECT == _EU_DAVEY_)	
//Define Rating Output(PWM duty) of each model
unsigned int LimitDutyTab[] = 
{
	PWM_MAX,PWM_MAX
};
unsigned int BoostDutyTab[] = 
{
	PWM_MAX,PWM_MAX
};

//Define Rating Output Current( x 20) of each model  
unsigned char LimitCurrentTab[] = 
{
	20,40
};
unsigned char BoostCurrentTab[] = 
{		
	25,50
};

#else
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
//Define Rating Output(PWM duty) of each model
unsigned int LimitDutyTab[] = 
{
	4500,5000,6000,7000,PWM_MAX,PWM_MAX,PWM_MAX
};
unsigned int BoostDutyTab[] = 
{
	5000,6000,7000,8000,PWM_MAX,PWM_MAX,PWM_MAX
};	
#else
//Define Rating Output(PWM duty) of each model
unsigned int LimitDutyTab[] = 
{
	300,400,580,780,980,PWM_MAX,PWM_MAX
};
unsigned int BoostDutyTab[] = 
{
	400,580,800,PWM_MAX,PWM_MAX,PWM_MAX,PWM_MAX
};	
#endif	
//Define Rating Output Current( x 20) of each model  
unsigned char LimitCurrentTab[] = 
{
	30,40,50,60,70,80,85
};
unsigned char BoostCurrentTab[] = 
{		
	38,50,63,75,88,100,100
};
#endif

//bool variable definition start
unsigned char bDoBoostModeFlag   = _FALSE;
unsigned char bOneSecondFlag     = _FALSE;
unsigned char bLEDFlashFlag      = _FALSE;
unsigned char bCalibrationFlag   = _FALSE;
unsigned char bLowOutputFlag     = _FALSE;
unsigned char bTimer3EnableFlag  = _FALSE;
unsigned char bADCDoneFlag       = _FALSE;
unsigned char bProtectFlag       = _FALSE;
unsigned char bLEDONFlag         = _FALSE;
unsigned char bADCUpdateFlag     = _FALSE;
#if(WATER_FLOW_CHECK == _ENABLE)
unsigned char bWFProcessFlag     = _FALSE;
#endif
#if(RUNNING_TIME_DISPLAY == _ENABLE) 
unsigned char bTimeAdjustFlag    = _FALSE;
unsigned char bReturnFlag        = _FALSE;
#endif
#if(CURRENT_CALIBRATION == _ENABLE)	
unsigned char bCalibrateMode     = _FALSE;
#endif
unsigned char bResetMode         = _FALSE;
unsigned char bFineTuneFlag      = _FALSE;
unsigned char bOutputFlag        = _FALSE;
unsigned char bStopOutputFlag    = _TRUE;
unsigned char bFastRevserseMode  = _FALSE;
//bool variable definition end

//char variable definition start
unsigned char ucModelNumber;
unsigned char ucCellPolarity;
unsigned char ucCellCurrent;
unsigned char ucPrevCellCurrent=0xFF;
unsigned char ucOriginalCurrent;
unsigned char ucMaxDisplayCurrent;
unsigned char ucBoostStatus = 0;
unsigned char bPermanentBoostMode;
unsigned char ucPermanentBoostCnt;
unsigned char ucADCCounter;
unsigned char ucDisplayWaitTimerCnt;
unsigned char ucPCBTestTimerCnt;
unsigned char ucFunctionMode;
unsigned char ucCellReverseSetting;
unsigned char ucCellReverseTime;
unsigned char ucTimer0Counter = 0;
unsigned char ucSecondCounter;
unsigned char bTestModeFlag;
//unsigned char ucWaitCounter;
unsigned char ucStableCnt;
unsigned char ucCurrentCalValue;
unsigned char ucFCurrentCalValue;
unsigned char ucRCurrentCalValue;
unsigned char ucConvertGain[2];
unsigned char ucPeriodCnt;

#if(WATER_FLOW_CHECK == _ENABLE)
unsigned char ucInWaterCounter = 0;
unsigned char ucWaterFlowCounter = 0;
unsigned char ucWaterFlowStatus=0xFF;
unsigned char ucNewWaterFlowStatus;
#endif
#if(RUNNING_TIME_DISPLAY == _ENABLE) 
unsigned char ucDigit;
unsigned char ucRunningMins;
unsigned char ucRunningHours[6];
unsigned char ucNumber[6];
#endif
#ifdef USE_DYNAMIC_ZERO_REFERENCE	
unsigned char ucCellZeroCurrent;
#endif
#if(CURRENT_CALIBRATION == _ENABLE)	
unsigned char ucSecretCode = 0;
unsigned char ucCalibrateTimeCnt = 0;
unsigned int ucGainValue;
#endif
unsigned char ucData[20];
unsigned char ucSaveType = _NOTHING;

#if(AC_POWER_CHECK == _ENABLE)
unsigned char bACPowerOffFlag = _FALSE;
unsigned char ucACPowerCounter = 0;
unsigned char ucACOnCounter = 0;
unsigned char ucACOffCounter = 0;
#endif

//char variable definition end

//int variable definition start
unsigned int  uiCellRealCurrent = 0;
unsigned int  uiLimitCurrent;
unsigned long iADCSum = 0;
unsigned int  iSaveCounter=0;
unsigned int  uiBoostCounter;
//unsigned int  uiLEDStatus;
unsigned int  uiLEDFlashStatus = 0;
unsigned long iADCData = 0;
unsigned int  uiMAX = 0;
unsigned int  uiMIN = 0xFFFF;
unsigned int  uiSum = 0;
unsigned int  uiStdH;
unsigned int  uiStdL;
unsigned int  ulSum = 0;

#if(CURRENT_CALIBRATION == _ENABLE)	
unsigned int  uiCurrentADC;
#endif
#if(SW_VERSION_DISPLAY	== _ENABLE)
unsigned char ucVersionNum;
unsigned char ucVersionNum2;
unsigned char Version[4];
#endif
unsigned int uiCurrentDuty = PWM_MIN;
unsigned int uiPWMHCnt = PWM_CNT_MIN;
unsigned int uiPWMLCnt = PWM_CNT_MAX;
unsigned int uiPWMPeriod = PWM_PERIOD;
unsigned int uiCurrentMaxDuty;
#if(BURN_IN_TEST == 1)
unsigned int uiOriginalDuty = PWM_MIN;
#endif
#if(DEBUG == _ENABLE)
unsigned int ucDutyDiv;
#endif
#if(BRAND_SELECT == _EU_DAVEY_)
#define OUTPUT_PERIOD   600
unsigned int uiOutputCounter = 0;
unsigned int uiOutputTime = 0;
#endif
//int variable definition end
void Deley_us(unsigned char ucCnt)
{
	delay10us(ucCnt);
}

void Delay_ms(unsigned int iCnt) // 
{
	delay1ms(iCnt);
}

#define FALSH_SAVE_START_ADDR         0x7000


/*******************************************************************************
 * FLASH 中断服务函数
 ******************************************************************************/
 void FlashInt(void)
 {
    if (TRUE == Flash_GetIntFlag(FlashPCInt))
    {
        Flash_ClearIntFlag(FlashPCInt);
        Flash_DisableIrq(FlashPCInt);
    }
    if (TRUE == Flash_GetIntFlag(FlashSlockInt))
    {
        Flash_ClearIntFlag(FlashSlockInt);
        Flash_DisableIrq(FlashSlockInt);
    }
      
 }

unsigned char Eeprom_ReadBytes(unsigned int uiAddr)
{
    unsigned char ucData;
	unsigned int u32Addr;
	u32Addr = FALSH_SAVE_START_ADDR;
	u32Addr += uiAddr;
	ucData = *((volatile uint8_t*)u32Addr);
	return ucData;
}

void Eeprom_WriteBytes(unsigned int uiAddr,unsigned char u8Data)
{
	unsigned int u32Addr;	
    en_result_t       enResult = Error;
	
	u32Addr = FALSH_SAVE_START_ADDR;
	u32Addr += uiAddr;
    ///< FLASH初始化（中断函数,编程时间,休眠模式配置）
    Flash_Init(FlashInt, 1, TRUE);
	
    ///< FLASH目标扇区擦除
    Flash_SectorErase(u32Addr);
    
    ///< FLASH 字节写、校验
    enResult = Flash_WriteByte(u32Addr, u8Data);
    if (Ok == enResult)
    {
        if(*((volatile uint8_t*)u32Addr) == u8Data)
        {
            enResult = Ok;
        }
        else
        {
        	enResult = Error;
            //return enResult;
        }
    }
    else
    {
        enResult = Error;
        //return enResult;
    }  	

}
void ReadModelNumber(void)
{
	ucModelNumber = Eeprom_ReadBytes(MODEL_NUMBER_ADDR);
}

void ReadCellPolarity(void)
{
	ucCellPolarity = Eeprom_ReadBytes(CELL_POLARITY_ADDR);
}

void ReadCellCurrentSetting(void)
{
	ucCellCurrent = Eeprom_ReadBytes(CELL_CURRENT_ADDR);
}

void ReadCellReverseSetting(void)
{
	ucCellReverseSetting = Eeprom_ReadBytes(CELL_REVERSE_SETTING_ADDR);
}

void ReadCellReverseTime(void)
{
	ucCellReverseTime = Eeprom_ReadBytes(CELL_REVERSE_TIME_ADDR);
}

void ReadPermanentBoostMode(void)
{
	bPermanentBoostMode = Eeprom_ReadBytes(PREMANENT_BOOST_ADDR);
}

void ReadTestModeFlag(void)
{
	bTestModeFlag = Eeprom_ReadBytes(TEST_MODE_FLAG_ADDR);
}
#if(CURRENT_CALIBRATION == _ENABLE)
void ReadCurrentCalValue(void)
{
	ucFCurrentCalValue = Eeprom_ReadBytes(CURRENT_CAL_ADDR);
	ucRCurrentCalValue = Eeprom_ReadBytes(R_CURRENT_CAL_ADDR);
}
#endif

#if(RUNNING_TIME_DISPLAY == _ENABLE)
void ReadCellRunningTime(void)
{
	unsigned char i;
	unsigned int k;
	ucRunningMins = Eeprom_ReadBytes(CELL_RUNNING_MINS_ADDR);
	k = CELL_RUNNING_TIME_ADDR;
	for(i=0;i<6;i++)
	{	
		ucRunningHours[i] = Eeprom_ReadBytes(k);
		k++;
	}	
}
#endif

void SaveCellPolarity(void)
{
	ucData[3] = ucCellPolarity;
	ucSaveType |= _USER_SETTING;		
}

void SaveCellCurrent(void)
{
    ucData[4] = ucCellCurrent;	
	ucSaveType |= _USER_SETTING;			
}

void SaveCellReverseTime(void)
{
	ucData[6] = ucCellReverseTime;
	ucSaveType |= _USER_SETTING;						
}

void SavePermanentBoostMode(void)
{
	ucData[7] = bPermanentBoostMode;
	ucSaveType |= _USER_SETTING;	
}

void SaveTestModeFlag(void)
{
	ucData[8] = bTestModeFlag;	
	ucSaveType |= _USER_SETTING;		
}

#if(CURRENT_CALIBRATION == _ENABLE)
void SaveCurrentCalValue(void)
{	
	ucData[9] = ucFCurrentCalValue;
	ucData[10] = ucRCurrentCalValue;	
	ucSaveType |= _USER_SETTING;		
}
void SaveCalValueByDirection(void)
{
	if(ucCellPolarity == _FORWARD)
	{
		//ucFCurrentCalValue = ucGainValue;
		//ucData[9] = ucFCurrentCalValue;
		ucData[9] = ucConvertGain[0];
		ucData[10] = ucConvertGain[1];
	}
	else
	{
		//ucRCurrentCalValue = ucGainValue;
		//ucData[10] = ucRCurrentCalValue;	
		ucData[11] = ucConvertGain[0];
		ucData[12] = ucConvertGain[1];		
	}
	ucSaveType |= _USER_SETTING;		
}
#endif

#if(RUNNING_TIME_DISPLAY == _ENABLE)
void ClearRunningData(void)
{
	unsigned char i;
	for(i = 13;i<20;i++ )
		ucData[i] = 0;
}

void SaveCellRunningMinutes(void)
{	
	ucData[13] = ucRunningMins;
	ucSaveType |= _RUNNING_DATA;		
}

void SaveCellRunningTime(void)
{	
	ucData[14] = ucRunningHours[0];
	ucData[15] = ucRunningHours[1];
	ucData[16] = ucRunningHours[2];
	ucData[17] = ucRunningHours[3];
	ucData[18] = ucRunningHours[4];
	ucData[19] = ucRunningHours[5];
	ucSaveType |= _RUNNING_DATA;		
}
#endif

void LEDControl(unsigned int ucID,unsigned char ucOnOff)
{
	if(ucOnOff == _ON)
	{
		switch (ucID)
		{
			case LED_11:		//D1
				Gpio_SetIO(GpioPortA, GpioPin8);
				break;			
			case LED_10:		//D2
				Gpio_SetIO(GpioPortB, GpioPin8);
				break;
			case LED_9:			//D3
				#if(BOARD_VERSION == BOARD_EVT_VERSION)		
				Gpio_SetIO(GpioPortA, GpioPin10);
				#else
				Gpio_SetIO(GpioPortB, GpioPin7);
				#endif
				break;
			case LED_8:			//D4
				#if(BOARD_VERSION == BOARD_EVT_VERSION)		
				Gpio_SetIO(GpioPortB, GpioPin13);
				#else
				Gpio_SetIO(GpioPortA, GpioPin9);
				#endif
				break;
			case LED_7:			//D5
				Gpio_SetIO(GpioPortB, GpioPin9);
				break;
			case LED_6:			//D6
				Gpio_SetIO(GpioPortB, GpioPin11);
				break;
			case LED_5:         //D7
				Gpio_SetIO(GpioPortB, GpioPin10);
				break;
			case LED_4:			//D8
				Gpio_SetIO(GpioPortB, GpioPin2);
				break;
			case LED_3:			//D9
				Gpio_SetIO(GpioPortB, GpioPin1);
				break;
			case LED_2:   		//D10
				Gpio_SetIO(GpioPortB, GpioPin0);
				break;
			case LED_1:			//D11
				Gpio_SetIO(GpioPortA, GpioPin7);
				break;		
			case LED_12:		//D12
				Gpio_SetIO(GpioPortA, GpioPin6);
				break;							
		}

	}
	else
	{		
		switch (ucID)
		{
			case LED_11:		//D1
				Gpio_ClrIO(GpioPortA, GpioPin8);
				break;			
			case LED_10:		//D2
				Gpio_ClrIO(GpioPortB, GpioPin8);
				break;
			case LED_9:			//D3
				#if(BOARD_VERSION == BOARD_EVT_VERSION)		
				Gpio_ClrIO(GpioPortA, GpioPin10);
				#else
				Gpio_ClrIO(GpioPortB, GpioPin7);
				#endif
				break;
			case LED_8:			//D4
				#if(BOARD_VERSION == BOARD_EVT_VERSION)		
				Gpio_ClrIO(GpioPortB, GpioPin13);
				#else
				Gpio_ClrIO(GpioPortA, GpioPin9);
				#endif
				break;
			case LED_7:			//D5
				Gpio_ClrIO(GpioPortB, GpioPin9);
				break;
			case LED_6:			//D6
				Gpio_ClrIO(GpioPortB, GpioPin11);
				break;
			case LED_5:			//D7
				Gpio_ClrIO(GpioPortB, GpioPin10);
				break;
			case LED_4:			//D8
				Gpio_ClrIO(GpioPortB, GpioPin2);
				break;
			case LED_3:			//D9
				Gpio_ClrIO(GpioPortB, GpioPin1);
				break;
			case LED_2:			//D10
				Gpio_ClrIO(GpioPortB, GpioPin0);
				break;
			case LED_1:			//D11
				Gpio_ClrIO(GpioPortA, GpioPin7);
				break;		
			case LED_12:		//D12
				Gpio_ClrIO(GpioPortA, GpioPin6);
				break;							
		}

	}
}

void LEDFlashSetting(unsigned int ucID,unsigned char ucOnOff)
{
	if(ucOnOff)
		uiLEDFlashStatus |= ucID;
	else
	{
		uiLEDFlashStatus &= ~ucID;
		LEDControl(ucID,_OFF);
	}
}

#if(SW_VERSION_DISPLAY	==_ENABLE)
void SWVersionDisplay(void)
{
	unsigned int iNum,iNum2;
	if(ucVersionNum != ucVersionNum2)
	{
		iNum2 = LED_1;
		iNum2 = iNum2<< ucVersionNum;
		LEDControl(iNum2,_ON);
		iNum = LED_1;
		iNum = iNum<< ucVersionNum2;
		LEDControl(iNum,_ON);
		Delay_ms(300);
		LEDControl(iNum,_OFF);
		Delay_ms(300);
		LEDControl(iNum,_ON);
		Delay_ms(300);
		LEDControl(iNum,_OFF);
		Delay_ms(300);		
		LEDControl(iNum,_ON);
		Delay_ms(300);
		//LEDControl(iNum,_OFF);
		Delay_ms(300);
		//LEDControl(iNum,_OFF);
		//LEDControl(iNum2,_OFF);
	}
	else
	{
		iNum = LED_1;
		iNum = iNum<< ucVersionNum;
		LEDControl(iNum,_ON);
		Delay_ms(300);
		LEDControl(iNum,_OFF);
		Delay_ms(300);
		LEDControl(iNum,_ON);
		Delay_ms(300);
		LEDControl(iNum,_OFF);
		Delay_ms(300);		
		LEDControl(iNum,_ON);
		Delay_ms(300);
		//LEDControl(iNum,_OFF);
		Delay_ms(300);			
	}
}
#endif

unsigned char DataRangeCheck(unsigned int uiAddr)
{

	unsigned char i;
	unsigned int k;

	k = uiAddr;
	for(i=0x00;i<0x0D;i++)
    {
		ucData[i] = Eeprom_ReadBytes(k);
		k++;
	}
	if((ucData[0] != Version[1])||(ucData[1] != Version[3]))
		return 1;
	if(ucData[2] > MODEL_NUMBER_MAX)
		return 1;
	if(ucData[3] > _FORWARD)
		return 1;
	if(ucData[4] > USER_MAX)
		return 1;
	if(ucData[5] > TIME_10HOURS)
		return 1;
	if(ucData[6] > TIME_10HOURS)
		return 1;
	if(ucData[7] > _ENABLE)
		return 1;
	if(ucData[8] > _ENABLE)
		return 1;	
	if(ucData[9] == 255)
		return 1;
	if(ucData[10] > 100)
		return 1;
	if(ucData[11] == 255)
		return 1;
	if(ucData[12] > 100)
		return 1;
	
	return 0;
}

//unsigned int u32Addr;	

void SaveDataToArea(unsigned int uiAddr,unsigned char ucLen,unsigned char* pucData)
{
	unsigned int u32Addr;	
	unsigned char ucIndex = 0;
    en_result_t  enResult = Error;
	unsigned char ucErrorCnt=0;

	ALLStop();
	
	u32Addr = FALSH_SAVE_START_ADDR;
	u32Addr += uiAddr;
    ///< FLASH初始化（中断函数,编程时间,休眠模式配置）
    Flash_Init(FlashInt, 1, TRUE);
	
    ///< FLASH目标扇区擦除
    Flash_SectorErase(u32Addr);


    ///< FLASH 字节写、校验
    for(ucIndex = 0;ucIndex<ucLen;ucIndex++)
    {
    	do
		{
		    enResult = Flash_WriteByte(u32Addr, pucData[ucIndex]);
			//delay10us(1);
		    if (Ok == enResult)
		    {
		        if(*((volatile uint8_t*)u32Addr) == pucData[ucIndex])
		        {
		            enResult = Ok;
					u32Addr++;
					//Gpio_ClrIO(GpioPortA, GpioPin6);
		        }
		        else
		        {
		        	enResult = Error;
					ucErrorCnt++;
					//Gpio_SetIO(GpioPortA, GpioPin6);
		        }
		    }
		    else
		    {
		        enResult = Error;
				ucErrorCnt++;
				//Gpio_SetIO(GpioPortA, GpioPin6);
		    } 
			if(ucErrorCnt >= 3)
			{
				ucErrorCnt = 0;
				//Gpio_SetIO(GpioPortA, GpioPin6);
				u32Addr++;
				enResult = Ok;
			}
    	}
		while(enResult == Error);
    }
	ALLStart();
}

void SaveModelNumber(void)
{
	ucData[2] = ucModelNumber;
	//ucSaveType |= _USER_SETTING;
	
	SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
	SaveDataToArea(DATA_BACKUP_ADDR,13,&ucData[0]);
}

void SaveCellReverseSetting(void)
{	
    ucData[5] = ucCellReverseSetting;	
	//ucSaveType |= _USER_SETTING;	
	SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
	SaveDataToArea(DATA_BACKUP_ADDR,13,&ucData[0]);
	
}

void SetToDefault(void)
{
	ucData[0] = Version[1];
	ucData[1] = Version[3];
	ucData[3] = DEFUALT_CELL_POLARITY;
	ucData[4] = DEFAULT_CELL_CURRENT;
	ucData[5] = DEFAULT_CELL_REVERSE;
	ucData[6] = 0;
	ucData[7] = 0;
	ucData[8] = 0;
	ucData[9] = 14;
	ucData[10] = 30;	
	ucData[11] = 16;
	ucData[12] = 20;			
}

void UpdateUserData(void)
{
	ucModelNumber = ucData[2];
	ucCellPolarity = ucData[3];
	ucCellCurrent = ucData[4];
	ucCellReverseSetting = ucData[5];
	ucCellReverseTime = ucData[6];
	bPermanentBoostMode = ucData[7];
	bTestModeFlag = ucData[8];
	ucFCurrentCalValue = ucData[9];
	ucRCurrentCalValue = ucData[10];
}

void UpdateRunningData(void)
{
	ucRunningMins = ucData[13];
	ucRunningHours[0] = ucData[14];
	ucRunningHours[1] = ucData[15];
	ucRunningHours[2] = ucData[16];
	ucRunningHours[3] = ucData[17];
	ucRunningHours[4] = ucData[18];
	ucRunningHours[5] = ucData[19];
}

void EepromDataCheck(void)
{
#if(SW_VERSION_DISPLAY	==_ENABLE)
	unsigned char i;
	unsigned int k;
	unsigned int uiTemp;
	const char *s;
#endif	

	unsigned char bNeedResetFlag;
	unsigned char bDataErrorFlag;
#if(SW_VERSION_DISPLAY	==_ENABLE)	
	s = SW;
	for(i= 0;i<4;i++)
	{	
		Version[i]=*s;
		s++;
	}
	k = 0x31;
	uiTemp = Version[1];
	ucVersionNum = uiTemp - k;
	uiTemp = Version[3];
	ucVersionNum2 = uiTemp -k;
#endif

	bNeedResetFlag = 0;

	bDataErrorFlag = DataRangeCheck(EEPROM_START_ADDRESS);

	if(bDataErrorFlag == 1)
	{
		if(DataRangeCheck(DATA_BACKUP_ADDR))
		{
			bNeedResetFlag = 1;
		}
		else
		{
			SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
			bNeedResetFlag = 0;
		}
	}
	else
	{
		bNeedResetFlag = 0;
	}
	
#if(RUNNING_TIME_DISPLAY == _ENABLE)
	k = CELL_RUNNING_MINS_ADDR;
	for(i=0;i<7;i++)
	{	  
		ucData[13+i] = Eeprom_ReadBytes(k);
		k++;
	}
	if((ucData[13] > 60)||(ucData[14] > 9)||(ucData[15] > 9)||(ucData[16] > 9)||(ucData[17] > 9)||(ucData[18] > 9)||(ucData[19] > 9))
	{
		ClearRunningData();
		SaveDataToArea(CELL_RUNNING_MINS_ADDR,7,&ucData[13]);
	}	
	UpdateRunningData();
#endif	

	if(bNeedResetFlag == 1)
	{
		ucData[2] = DEFUALT_MODEL;
		SetToDefault();
		SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
		SaveDataToArea(DATA_BACKUP_ADDR,13,&ucData[0]);
#if(SW_VERSION_DISPLAY	==_ENABLE)        
		SWVersionDisplay();
#endif    
	}
	UpdateUserData();

}

void SerivceReset(void)
{
	SetToDefault(); //Set all to default except model number
	SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
	SaveDataToArea(DATA_BACKUP_ADDR,13,&ucData[0]);
	UpdateUserData();
	ucBoostStatus = 0;
	bDoBoostModeFlag = 0;
	//bBoostSetupFlag = 0;
	bPermanentBoostMode = 0;

}

void FactoryReset(void)
{
	SerivceReset();

#if(RUNNING_TIME_DISPLAY == _ENABLE)	
	ClearRunningData();
	SaveDataToArea(CELL_RUNNING_MINS_ADDR,7,&ucData[13]);
	UpdateRunningData();
#endif

}

void LEDFlashDisplay(void)
{
	unsigned char i;
	unsigned int j=LED_1;

	if(uiLEDFlashStatus == 0)
		return;
	if(bLEDFlashFlag)
	{
		for(i=0;i<12;i++)
		{
			if(uiLEDFlashStatus & j)
			{
				//if(uiLEDStatus & j)
				if(bLEDONFlag)
				{
					LEDControl(j,_OFF);
				}
				else
				{
					LEDControl(j,_ON);					
				}
			}				
			j=j<<1;
		}
		if(bLEDONFlag)
			bLEDONFlag = 0;
		else
			bLEDONFlag = 1;
		bLEDFlashFlag = 0;
	}

}
#if(RUNNING_TIME_DISPLAY == _ENABLE)
void LEDALLControl(unsigned char ucOnOff)
{
	if(ucOnOff)
	{
		Gpio_SetIO(GpioPortA, GpioPin8);
		Gpio_SetIO(GpioPortB, GpioPin8);
	#if(BOARD_VERSION == BOARD_EVT_VERSION)		
		Gpio_SetIO(GpioPortA, GpioPin10);
		Gpio_SetIO(GpioPortB, GpioPin13);
	#else
		Gpio_SetIO(GpioPortB, GpioPin7);
		Gpio_SetIO(GpioPortA, GpioPin9);	
	#endif
		Gpio_SetIO(GpioPortB, GpioPin9);
		Gpio_SetIO(GpioPortB, GpioPin11);
		Gpio_SetIO(GpioPortB, GpioPin10);
		Gpio_SetIO(GpioPortB, GpioPin2);
		Gpio_SetIO(GpioPortB, GpioPin1);
		Gpio_SetIO(GpioPortB, GpioPin0);
		Gpio_SetIO(GpioPortA, GpioPin7);
		Gpio_SetIO(GpioPortA, GpioPin6);

	}
	else
	{
		Gpio_ClrIO(GpioPortA, GpioPin8);
		Gpio_ClrIO(GpioPortB, GpioPin8);
	#if(BOARD_VERSION == BOARD_EVT_VERSION)		
		Gpio_ClrIO(GpioPortA, GpioPin10);
		Gpio_ClrIO(GpioPortB, GpioPin13);
	#else
		Gpio_ClrIO(GpioPortB, GpioPin7);
		Gpio_ClrIO(GpioPortA, GpioPin9);	
	#endif
		Gpio_ClrIO(GpioPortB, GpioPin9);
		Gpio_ClrIO(GpioPortB, GpioPin11);
		Gpio_ClrIO(GpioPortB, GpioPin10);
		Gpio_ClrIO(GpioPortB, GpioPin2);
		Gpio_ClrIO(GpioPortB, GpioPin1);
		Gpio_ClrIO(GpioPortB, GpioPin0);
		Gpio_ClrIO(GpioPortA, GpioPin7);
		Gpio_ClrIO(GpioPortA, GpioPin6);
	}
}
#endif
#if(SUPPORT_REVERSE_SETTING == _ENABLE)
void ReverseLEDDisplay(unsigned char ucOnOff)
{
	LEDControl(LED_1,ucOnOff);
	if(ucCellReverseSetting == TIME_10HOURS)
	{
		LEDControl(LED_3,ucOnOff);
		LEDControl(LED_2,ucOnOff);
	}
	if(ucCellReverseSetting == TIME_7P5HOURS)
		LEDControl(LED_2,ucOnOff);
}
#endif
void PowerOffLED(void)
{
	uiLEDFlashStatus = 0;
	//uiLEDStatus = 0;
	LEDALLControl(_OFF);
}

#if(SUPPORT_REVERSE_SETTING == _ENABLE)
void PowerOnLED(void)
{
	unsigned char i;
	for(i=0;i<3;i++)
	{
    	ReverseLEDDisplay(_ON);
    	Delay_ms(300);
    	ReverseLEDDisplay(_OFF);
    	Delay_ms(150);
	}
}
#endif

#if(CURRENT_CALIBRATION == _ENABLE)	
void LEDDisplayHexValue(unsigned int uiVal)
{
	unsigned char i;
	unsigned int j,k;
	j = LED_1;
	k = LED_1;
	for(i=0;i<10;i++)
	{		
		if(uiVal&j)
		{			
			LEDControl(k,_ON);
		}
		else
		{			
			LEDControl(k,_OFF);
		}
		j=j<<1;
		k=k<<1;
	}

}
#endif

void CellCurrentLED(unsigned char ucVal)
{
	unsigned char ucLEDNum,i;
	unsigned int j;
	
#if 0//(CURRENT_TEST == 1) 
	j = LED_1;

	for(i=0;i<8;i++)
	{
		
		if(ucVal&j)
		{			
			LEDControl(j,_ON);
			j=j<<1;
		}
		else
		{			
			LEDControl(j,_OFF);
			j=j<<1;
		}

	}
#else
	ucLEDNum = ucVal;

	j = LED_1;
	for(i =0;i<ucLEDNum;i++)
	{
		LEDControl(j,_ON);
		j=j<<1;
	}
	if(ucLEDNum<10)
	{
		for(i=ucLEDNum;i<10;i++)
		{
			LEDControl(j,_OFF);
			j=j<<1;
		}
	}

#endif
}

void ModelChangeLED(void)
{	
#if(BRAND_SELECT == _EU_DAVEY_)
	CellCurrentLED(0);

	if(ucModelNumber == MODEL_8G_2A)
		LEDControl(LED_8,_ON);
	else
	{	
		LEDControl(LED_2,_ON);	
		LEDControl(LED_4,_ON);
	}
#else
	unsigned int j;
	CellCurrentLED(0);

	if(ucModelNumber == 4)
		LEDFlashSetting(LED_5,_ON);
	else
	{
		LEDFlashSetting(LED_5,_OFF);
		LEDControl(LED_5,_ON);
		j = LED_1;

		j = j<<ucModelNumber;	
		
		LEDControl(j,_ON);
	}
#endif	
}

void GetModelRatingCurrent(void)
{
	if((bPermanentBoostMode)||(bDoBoostModeFlag))
	{
		uiCurrentMaxDuty = BoostDutyTab[ucModelNumber];
		ucMaxDisplayCurrent =  BoostCurrentTab[ucModelNumber];
	}
	else
	{
		uiCurrentMaxDuty = LimitDutyTab[ucModelNumber];
		ucMaxDisplayCurrent =  LimitCurrentTab[ucModelNumber];
	}

	uiLimitCurrent = 10;
	uiLimitCurrent = uiLimitCurrent * ucMaxDisplayCurrent;
	#if(DEBUG == _ENABLE)
	ucDutyDiv = uiCurrentMaxDuty/10;
	#endif	
}


#ifdef USE_DYNAMIC_ZERO_REFERENCE
void GetZeroCurrentReference(void)
{
	unsigned char iTemp,iSum=0;
	unsigned char i;
	for(i=0;i<10;i++)
	{		
		iTemp = 1;	
		iSum+=iTemp;
	}

	ucCellZeroCurrent = (iSum/5);

}
#endif

void GetCellRealCurrent(void)
{
	unsigned long uiTemp;
	unsigned int uiTemp2;
	if(bStopOutputFlag)
	{
		ucStableCnt = 0;
		uiMIN = 0xFFFF;
		uiMAX = 0;
		ulSum = 0;	
		bADCUpdateFlag = _TRUE;
		uiCellRealCurrent = 0;
		iADCData = 0;		
		iADCSum = 0;
		ucADCCounter = 0;	
		return;
	}
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION) 
	if(bADCDoneFlag)
	{
		uiTemp = iADCData/ADCMAXCNT;
		if(uiTemp > uiMAX)
			uiMAX = uiTemp;
		if(uiTemp < uiMIN)
			uiMIN = uiTemp;
		ulSum += uiTemp;
		ucStableCnt++;

		if(ucStableCnt == ADC_STABLE_NUM)
		{
			ucStableCnt = 0;
			ulSum -= uiMAX;
			ulSum -= uiMIN;
			uiTemp = ulSum / ADC_STABLE_NUM2;
			if(uiTemp>=2048)//1.65V
			uiCurrentADC = uiTemp-2035;//2017;
			else if(uiTemp<=2017)//1.625V
			uiCurrentADC = 2048-uiTemp;	
			else
			uiCurrentADC = (2048-uiTemp)/2;		
			//I   = (ADC * Vref)/(4096*Gain*Rs)
			//Vef = 3.3V, Rs  = 1mR = 0.001R
			uiTemp = ucConvertGain[0];
			uiTemp = uiTemp*uiCurrentADC;
			uiTemp2 = ucConvertGain[1];
			uiTemp2 = uiTemp2*uiCurrentADC;
			uiTemp2 = uiTemp2/100;
			uiTemp += uiTemp2;

			uiCellRealCurrent = (uiTemp/10);	
			uiMIN = 0xFFFF;
			uiMAX = 0;
			ulSum = 0;		
			bADCUpdateFlag = _TRUE;
	#if(EMC_TEST == _ENABLE)
			uiCellRealCurrent = 0;
	#endif
		}

		iADCData = 0;
		bADCDoneFlag = _FALSE;
	}

#else
	if(bADCDoneFlag)
	{
		uiTemp = iADCData/ADCMAXCNT;
		uiSum += uiTemp;
		ucPeriodCnt++;
		if(ucPeriodCnt == 10)
		{
			uiTemp = uiSum/10;
			if(uiTemp > uiMAX)
				uiMAX = uiTemp;
			if(uiTemp < uiMIN)
				uiMIN = uiTemp;
			ucPeriodCnt = 0;
			ulSum += uiTemp;
			ucStableCnt++;
			uiSum =0;
		}

		if(ucStableCnt == ADC_STABLE_NUM)
		{
			ucStableCnt = 0;
			ulSum -= uiMAX;
			ulSum -= uiMIN;
			uiTemp = ulSum / ADC_STABLE_NUM2;
			uiCurrentADC = uiTemp;
			//I   = (ADC * Vref)/(4096*Gain*Rs)
			//Vef = 3.3V, Rs  = 5mR = 0.005R
			//ulTemp = 8057; //  (660/4096)*100000/2
			//ulTemp = ulTemp*uiCurrentADC;
			uiTemp = ucConvertGain[0];
			uiTemp = uiTemp*uiCurrentADC;
			uiTemp2 = ucConvertGain[1];
			uiTemp2 = uiTemp2*uiCurrentADC;
			uiTemp2 = uiTemp2/10;
			uiTemp += uiTemp2;
			uiCellRealCurrent = (uiTemp/100);	
			uiMIN = 0xFFFF;
			uiMAX = 0;
			ulSum = 0;		
			bADCUpdateFlag = _TRUE;
			#if(EMC_TEST == _ENABLE)
			uiCellRealCurrent = 0;
			#endif
		}
		iADCData = 0;
		bADCDoneFlag = _FALSE;
	}	
#endif	
}

void CellRealCurrentDisplay(void)
{
	unsigned int iTemp;
	unsigned char ucLEDNum;

	uiLEDFlashStatus &= 0xFC00;
	if(bProtectFlag == _TRUE)
		return;
#if(CURRENT_CALIBRATION == _ENABLE)	
    #if(BURN_IN_TEST == 1)
	bCalibrateMode = 1;
    #endif

	if(bCalibrateMode)
	{
		LEDDisplayHexValue(uiCurrentADC);
		//LEDDisplayHexValue(uiCellRealCurrent);
		//LEDDisplayHexValue(ucCurrentCalValue);
		//LEDDisplayHexValue(ucGainValue);
	}
	else
#endif
	{
	#if(DIRECTION_TEST == 1)
		if(CURRENT_CTRL)
			uiCellRealCurrent = 0xFFFF;
		else
			uiCellRealCurrent = 0;
	#endif
	
		if(ucCellCurrent == USER_MIN)
			uiCellRealCurrent = 0;
	
		iTemp = uiCellRealCurrent;
	    iTemp += 20;
		iTemp = iTemp *10;
		ucLEDNum =(unsigned char)(iTemp/uiLimitCurrent);
		if(ucLEDNum >10)
			ucLEDNum = 10;
	#if(WATER_FLOW_CHECK == _ENABLE)
		if(ucWaterFlowStatus)
			ucLEDNum = 0;
	#endif	
	#if(DEBUG == _DISABLE)
	if(ucLEDNum!=IOTuiCellRealCurrent)
	{
		IOTuiCellRealCurrent=ucLEDNum;
		eIOTEVENT|=_IOT_EVENT_UPDATE_CURRENT;
		uiIOTEVENTCURRENTCNT=500;
	}
	CellCurrentLED(ucLEDNum);	
	#endif
	//LEDDisplayHexValue(uiOutputCounter);
	}

}
void CalculateConvertValue(void)
{
	unsigned long uiTemp;
	unsigned  int uiTemp2;
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)
	uiTemp = 80566;
	uiTemp = uiTemp/ucCurrentCalValue;
#else
	uiTemp = 805664;
	uiTemp2 = ucGainValue;
	uiTemp = uiTemp/uiTemp2;
#endif
	ucConvertGain[0] = uiTemp/100;
	uiTemp2 = 100;
	uiTemp2 = uiTemp2 * ucConvertGain[0];
	ucConvertGain[1] = uiTemp - uiTemp2;
}
void CellPolarityControl(void)
{	
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
	if(CURRENT_CTRL == _LOW)
#else
	if(CURRENT_CTRL == _HIGH)
#endif		
	{
		CURRENT_CTRL_OFF;
		Delay_ms(500);
	}
	if(ucCellPolarity)//Forward
	{
		//ucCurrentCalValue = ucFCurrentCalValue;
		ucConvertGain[0] = ucData[9];
		ucConvertGain[1] = ucData[10];
		CELL_POLARITY_LOW;
		CELL_POLARITY2_LOW;
		Deley_us(5);
		CELL_POLARITY_HIGH;
		LEDFlashSetting(LED_11,_OFF);
	}
	else//Reverse
	{
		//ucCurrentCalValue = ucRCurrentCalValue;
		ucConvertGain[0] = ucData[11];
		ucConvertGain[1] = ucData[12];	
		CELL_POLARITY_LOW;
		CELL_POLARITY2_LOW;
		Deley_us(5);
		CELL_POLARITY2_HIGH;		
		LEDFlashSetting(LED_11,_ON);
	}
	
	LEDControl(LED_11,_ON);
	//CalculateConvertValue();
}

void CellPolarityReverse(void)
{
	unsigned char ucPol;
	ucPol = Eeprom_ReadBytes(CELL_POLARITY_ADDR);
	if(ucPol == _FORWARD)
		//Eeprom_WriteBytes(CELL_POLARITY_ADDR,_REVERSE);
		ucData[3] = _REVERSE;
	else
		//Eeprom_WriteBytes(CELL_POLARITY_ADDR,_FORWARD);
		ucData[3] = _FORWARD;
	ucSaveType |= _USER_SETTING;
}

#if((POWER_BOARD_TEST == _ENABLE)||(IOT_TEST==_ENABLE))
void CellReverse(void)
{
	if(ucCellPolarity)
		ucCellPolarity = 0;
	else
		ucCellPolarity = 1;
	Delay_ms(200);
	CellPolarityControl();
}
#endif
void CalculateThresholdLimit(void)
{
	unsigned int iTemp;
	iTemp =ucMaxDisplayCurrent;
#if(BRAND_SELECT == _AU_CAS_)	
    iTemp =iTemp * ucCellCurrent;
#else
	iTemp =iTemp * USER_MAX;
#endif
	uiStdL = iTemp/10;	// 
	uiStdH = 10;
	uiStdH += uiStdL;
}

void CellCurrentOutput(unsigned char ucVal)
{
	if(bProtectFlag)
		return;	
#if(WATER_FLOW_CHECK == _ENABLE)	
	if(ucWaterFlowStatus == 1)
		ucVal = 0;
#endif	
	if(ucPrevCellCurrent != ucVal)
	{
		ucPrevCellCurrent = ucVal;
	}
	else
		return;
	
    if(ucVal == USER_MIN)
    { 
    
		PWMStop();
    	uiCurrentDuty = PWM_MIN;
    	uiPWMHCnt = PWM_CNT_MIN;
		uiPWMLCnt = PWM_CNT_MAX;		
		CURRENT_CTRL_OFF;
		bTimer3EnableFlag = _FALSE;
		bOutputFlag = _DISABLE;
		bStopOutputFlag = _TRUE;
	}
	#if(BURN_IN_TEST == 1)
	else if(ucVal == USER_MAX)
	{		
		//CURRENT_CTRL_ON;
		;//bTimer3EnableFlag = _FALSE;
	}
	#endif
	else
	{									
		if(bOutputFlag == _FALSE)
		{
			bOutputFlag = _ENABLE;
			bTimer3EnableFlag = _TRUE;
		}
		if((bStopOutputFlag == _TRUE)&&(ucWaterFlowStatus == 0)&&(bProtectFlag == _FALSE))
		{
			bStopOutputFlag = _FALSE;
			uiOutputCounter = 0;			
			CURRENT_CTRL_OFF;
			PWMStart();
		}
	}

}

void CellOutputCalibration(void)
{
#if(BURN_IN_TEST == 1)

	unsigned int uiTemp;
#else
	unsigned int iTempSum;	
	unsigned int uiTol,ucStep;
	unsigned int uiStdHigh,uiStdLow; 	
#endif

    if(ucCellCurrent == 0)
		return;
	if(bProtectFlag)
		return;
	if(bResetMode)
		return;
#if(WATER_FLOW_CHECK == _ENABLE)
	if(ucWaterFlowStatus)	
		return;
#endif	
#if(BRAND_SELECT == _EU_DAVEY_)
	if(bStopOutputFlag)
		return;
#endif	
#if(BURN_IN_TEST == 1)
	if(ucPCBTestTimerCnt <= 11)
	{
		if(uiCurrentDuty > uiOriginalDuty)
		{
			uiCurrentDuty--;
			uiPWMHCnt = uiCurrentDuty;//uiPWMHCnt = uiCurrentDuty *10;
			uiPWMLCnt =PWM_CNT_MAX - uiPWMHCnt;
		}
		else if(uiCurrentDuty < uiOriginalDuty)
		{
			uiTemp = uiOriginalDuty;
			uiTemp -= uiCurrentDuty;
			if(uiTemp >= 5)
			{
			    uiCurrentDuty+=5;
			}
			else
			{
				uiCurrentDuty+=uiTemp;
			}
			uiPWMHCnt = uiCurrentDuty;//uiPWMHCnt = uiCurrentDuty *10;
			uiPWMLCnt =PWM_CNT_MAX - uiPWMHCnt;		
		}			
	}
#else
    #if(CURRENT_CALIBRATION == _ENABLE)	
	if(bCalibrateMode)
	{
		return;
	}
	#endif

	if(bFineTuneFlag)
	{
		uiStdHigh = uiStdH;
		uiStdLow = uiStdL;
	}
	else
	{
		uiStdHigh = uiStdH ;//+ 5;
		uiStdLow = uiStdL -10;
	}

	if(uiCellRealCurrent>uiStdHigh)
	{
		bFineTuneFlag = _TRUE;
		uiTol = uiCellRealCurrent - uiStdH;
		if(uiTol > 200)		//2A
			ucStep = 40;
		else if(uiTol > 100) //1A
			ucStep = 20;
		else if(uiTol > 50) //0.5A
			ucStep = 10;
		else
			ucStep = 5;
		iTempSum = PWM_MIN;
		#if(SUPPORT_1KHZ == _DISABLE)		
		if(uiStdL <= 200)	//2A
		ucStep = ucStep*5;
		else		
		ucStep = ucStep *10;
		#endif		
		iTempSum += ucStep;
		if(uiCurrentDuty >= iTempSum)
		{
			uiCurrentDuty -= ucStep;
			uiPWMHCnt = uiCurrentDuty;//uiPWMHCnt = uiCurrentDuty *10;
			uiPWMLCnt =PWM_CNT_MAX - uiPWMHCnt;
			if(bTimer3EnableFlag == 0)
			{		
				CURRENT_CTRL_OFF;
				bTimer3EnableFlag = _TRUE;
				PWMStart();
			}
		}
		else
		{
			PWMStop();
			uiCurrentDuty = PWM_MIN;
			uiPWMHCnt = PWM_CNT_MIN;
			uiPWMLCnt = PWM_CNT_MAX;
			if(bTimer3EnableFlag)
			{
				bTimer3EnableFlag = _FALSE; 			
				CURRENT_CTRL_OFF;				
			}
		}
	}
	else if(uiCellRealCurrent<uiStdLow)
	{
		bFineTuneFlag = _TRUE;
		uiTol = uiStdL - uiCellRealCurrent;
		if(uiTol > 200)		//2A
			ucStep = 40;
		else if(uiTol > 100) //1A
			ucStep = 20;
		else if(uiTol > 50) //0.5A
			ucStep = 10;
		else
			ucStep = 5;
		iTempSum = uiCurrentDuty;
	
		#if(SUPPORT_1KHZ == _DISABLE)
		if(uiStdL <= 200)	//2A
		ucStep = ucStep*5;
		else
		ucStep = ucStep *10;
		#endif
		iTempSum += ucStep;
		if(iTempSum <= uiCurrentMaxDuty)
		{
			uiCurrentDuty = iTempSum;
			uiPWMHCnt = uiCurrentDuty;//uiPWMHCnt = uiCurrentDuty *10;
			uiPWMLCnt =PWM_CNT_MAX - uiPWMHCnt;
		}
		else
		{
			uiCurrentDuty = uiCurrentMaxDuty;
			uiPWMHCnt = uiCurrentDuty;//uiPWMHCnt = uiCurrentDuty *10;
			uiPWMLCnt =PWM_CNT_MAX - uiPWMHCnt;
		}
		if(bTimer3EnableFlag == 0)
		{		
			CURRENT_CTRL_OFF;
			bTimer3EnableFlag = _TRUE;
			PWMStart();
		}
	}
	else
	{		
		bFineTuneFlag = _FALSE; 
	}
#endif
#if(DEBUG == _ENABLE)
	LEDDisplayHexValue(uiPWMHCnt);
	//LEDDisplayHexValue(uiCurrentADC);
#endif
}

void CellOutputCurrentAdjust(unsigned char ucAdjDirect)
{
	unsigned char ucNum;
	unsigned int iTemp;
	unsigned int uiLedNum;
	if((ucBoostStatus)||(bDoBoostModeFlag))
		return;

	if(bResetMode)
	{
		if(ucAdjDirect == 1)
		{
			SerivceReset();
			LEDControl(LED_11,_ON);
			Delay_ms(100);
			LEDControl(LED_11,_OFF);
			Delay_ms(100);
			LEDControl(LED_11,_ON);
			Delay_ms(100);
			LEDControl(LED_11,_OFF);
			Delay_ms(100);
			LEDControl(LED_11,_ON);
			Delay_ms(100);
			LEDControl(LED_11,_OFF);
			Delay_ms(100);	
			LEDFlashSetting(LED_11,_ON);
		}
		return;
	}

	
#if(DIRECTION_TEST == 1)
	if(ucAdjDirect == 1)
	{
		CellCurrentLED(10);
		CURRENT_CTRL_ON;
		ucDisplayWaitTimerCnt = 3;	
	}
	else if(ucAdjDirect == 2)
	{
		CellCurrentLED(0);
		CURRENT_CTRL_OFF;
		ucDisplayWaitTimerCnt = 3;	
	}
	return;
#endif
    
#if(CURRENT_CALIBRATION == _ENABLE)	
	#if(BURN_IN_TEST == 1)
		bCalibrateMode = 1;
	#endif
    if(bCalibrateMode)
    {
	    if(ucAdjDirect == 1)
		{
		#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
		
			if(uiPWMHCnt < PWM_MAX)
			{
				uiPWMHCnt+= 50;
				CellCurrentOutput(50);
			}
			else
			{
				uiPWMHCnt = PWM_MAX;
			}
		#else
			if(uiPWMHCnt<uiPWMPeriod)
			{
				uiPWMHCnt++;
				uiPWMLCnt--;
				CellCurrentOutput(50);
			}
			else
			{
				CellCurrentOutput(USER_MAX);
			}
		#endif	
            #if(BURN_IN_TEST == 1)
			uiOriginalDuty = uiPWMHCnt;//uiOriginalDuty = uiPWMHCnt/10;
            #endif
		}
		else if(ucAdjDirect == 2)
		{
		#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
			if(uiPWMHCnt >= (PWM_MIN+50))
			{
				uiPWMHCnt-= 50;
				CellCurrentOutput(50);
			}
			else
			{
				CellCurrentOutput(USER_MIN);
			}				
		#else		
			if(uiPWMHCnt>PWM_MIN)
			{
				uiPWMHCnt--;
				uiPWMLCnt++;
				CellCurrentOutput(50);
			}	
			else
			{
				CellCurrentOutput(USER_MIN);
			}
		#endif
            #if(BURN_IN_TEST == 1)
			uiOriginalDuty = uiPWMHCnt;//uiOriginalDuty = uiPWMHCnt/10;
            #endif
		}
		#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
		iTemp = uiPWMHCnt/10;
		LEDDisplayHexValue(iTemp);
		#else
		LEDDisplayHexValue(uiPWMHCnt);
		#endif
		
		ucDisplayWaitTimerCnt = 3;	
	}
	else
#endif
	{
		uiLEDFlashStatus &= 0xFC00;
		if(ucAdjDirect == 1)
		{
			if(ucCellCurrent<USER_MAX)
				ucCellCurrent+=5;
			#if(DEBUG == _ENABLE)
			uiCurrentMaxDuty = ucDutyDiv*ucCellCurrent;
			#if(SUPPORT_1KHZ == _ENABLE)
			uiCurrentMaxDuty = uiCurrentMaxDuty/10;
			#endif
			#endif
		}
		else if(ucAdjDirect == 2)
		{
			if(ucCellCurrent>5)
				ucCellCurrent-=5;
			else
				ucCellCurrent = USER_MIN;
			#if(DEBUG == _ENABLE)
			uiCurrentMaxDuty = ucDutyDiv*ucCellCurrent;
			#if(SUPPORT_1KHZ == _ENABLE)
			uiCurrentMaxDuty = uiCurrentMaxDuty/10;
			#endif
			#endif		
		}
		uiOutputTime = 6*ucCellCurrent;
		iTemp = ucCellCurrent;
		ucNum = 10;
		ucNum = iTemp/ucNum;
		//uiOutputTime = 6*ucNum;
		iTemp = ucNum *10;
		CellCurrentLED(ucNum);
		if(ucCellCurrent>iTemp)
		{
			uiLedNum = LED_1<<ucNum;
			LEDFlashSetting(uiLedNum,_ON);
			LEDControl(uiLedNum,_ON);
		}
		CellCurrentOutput(ucCellCurrent);

		if(ucAdjDirect != 0)
		{
			uiOutputCounter = 0;
			SaveCellCurrent();
			#if(BRAND_SELECT == _AU_CAS_)
			CalculateThresholdLimit();
			#endif
			ucDisplayWaitTimerCnt = 5;	
		}		
	}
	
}

void CellOutputCurrentAdjustAPI(unsigned char CellCurrent)
{
	unsigned char ucNum;
	unsigned int iTemp;
	unsigned int uiLedNum;
	ucCellCurrent=CellCurrent;
	if((ucBoostStatus)||(bDoBoostModeFlag))
		return;

	if(bResetMode)
	{
		return;
	}
	{
		uiLEDFlashStatus &= 0xFC00;
		#if(DEBUG == _ENABLE)
		uiCurrentMaxDuty = ucDutyDiv*ucCellCurrent;
		#if(SUPPORT_1KHZ == _ENABLE)
		uiCurrentMaxDuty = uiCurrentMaxDuty/10;
		#endif
		#endif
		uiOutputTime = 6*ucCellCurrent;
		iTemp = ucCellCurrent;
		ucNum = 10;
		ucNum = iTemp/ucNum;
		//uiOutputTime = 6*ucNum;
		iTemp = ucNum *10;
		CellCurrentLED(ucNum);
		if(ucCellCurrent>iTemp)
		{
			uiLedNum = LED_1<<ucNum;
			LEDFlashSetting(uiLedNum,_ON);
			LEDControl(uiLedNum,_ON);
		}
		CellCurrentOutput(ucCellCurrent);
		uiOutputCounter = 0;
		SaveCellCurrent();
#if(BRAND_SELECT == _AU_CAS_)
		CalculateThresholdLimit();
#endif
		ucDisplayWaitTimerCnt = 5;	
	}
}


void CellOutputControl(void)
{
	
	unsigned char i;
	unsigned char ucTemp;

	GetModelRatingCurrent();

	#if(DIRECTION_TEST == 1)
	CellCurrentLED(10);
	CURRENT_CTRL_ON;	
	#endif

	ucTemp = 0;
    i = 0;
	do
	{
		Delay_ms(50);
		ucTemp+=10;
		CellCurrentLED(i);
		i++;
	}
	while (ucTemp<=ucCellCurrent);
	uiOutputTime = 6*ucCellCurrent;
	//uiOutputTime = 6*i;
	//CellCurrentOutput(ucCellCurrent);
	CalculateThresholdLimit();

	//Delay_ms(200);
	
	ucDisplayWaitTimerCnt = 4;	//Waiting 4 Second than start display the current
}

#if(SUPPORT_REVERSE_SETTING == _ENABLE)
void ReverseTimeSettingKeyProcess(unsigned char ucKey)
{
#if(SUPPORT_FAST_REVERSE == _ENABLE)
	#if(BRAND_SELECT == _AU_CAS_)
	#else
		if(ucKey == KEY_UP_DOWN)
		{
			bFastRevserseMode = _TRUE;
		}
		if(bFastRevserseMode == _TRUE)
		{
			if(ucKey == KEY_UP)
			{
				if(ucCellReverseSetting == TIME_8HOURS)
				{
					ucCellReverseSetting = TIME_2HOURS;
					CellCurrentLED(2);
					ucDisplayWaitTimerCnt = 11;
					SaveCellReverseSetting();
				}				
			}
			else if(ucKey == KEY_DOWN)	
			{
				if(ucCellReverseSetting == TIME_2HOURS)
				{
					ucCellReverseSetting = TIME_1HOURS;
					CellCurrentLED(1);
					ucDisplayWaitTimerCnt = 11;
					SaveCellReverseSetting();
				}
			}
		}
	#endif	
#else
	#if(BRAND_SELECT == _AU_CAS_)
	if(ucKey == KEY_UP)
	{
		if(ucCellReverseSetting == TIME_10HOURS)
		{
			ucCellReverseSetting = TIME_7P5HOURS;
			LEDFlashSetting(LED_3,_OFF);
			LEDControl(LED_3, _OFF);
			ucDisplayWaitTimerCnt = 11;
			SaveCellReverseSetting();
		}
	}
	else if(ucKey == KEY_DOWN)
	{
		if(ucCellReverseSetting == TIME_7P5HOURS)
		{
			ucCellReverseSetting = TIME_5HOURS;
			LEDFlashSetting(LED_2,_OFF);
			LEDControl(LED_2, _OFF);
			ucDisplayWaitTimerCnt = 11;
			SaveCellReverseSetting();
		}
	}	
	#endif
#endif
}
#endif

void ModelSelectKeyProcess(unsigned char ucKey)
{
	if(ucKey == KEY_UP)
	{
		if(ucModelNumber < MODEL_NUMBER_MAX)
			ucModelNumber ++;
		else
			ucModelNumber = MODEL_NUMBER_MIN; 
	}
	else if(ucKey == KEY_DOWN)
	{
		if(ucModelNumber > MODEL_NUMBER_MIN)
			ucModelNumber--;
		else
			ucModelNumber = MODEL_NUMBER_MAX;

	}
	ucDisplayWaitTimerCnt = 11;
	ModelChangeLED();
	SaveModelNumber();	
#if(SUPPORT_REVERSE_SETTING == _ENABLE)	
#if(BRAND_SELECT == _AU_CAS_)	
	ucCellReverseSetting = TIME_7P5HOURS;
	SaveCellReverseSetting();	
#endif	
#endif
}
#if(RUNNING_TIME_DISPLAY == _ENABLE)
void RunningLEDDisplay(void)
{
	unsigned char ucLEDNum,i;
	unsigned int j;

	if(ucFunctionMode != 3)
		return;
	if(bReturnFlag)
	{
		if(ucDisplayWaitTimerCnt == 1)
		{
			LEDALLControl(_ON);
			Delay_ms(100);
			LEDALLControl(_OFF);
			Delay_ms(100);
			LEDALLControl(_ON);
			Delay_ms(100);
			LEDALLControl(_OFF);
			Delay_ms(100);
			LEDALLControl(_ON);
			Delay_ms(100);
			ucDisplayWaitTimerCnt = 0;
			bReturnFlag = _FALSE;
		}
		return;
	}
	if(ucDisplayWaitTimerCnt == 6)
	{
		LEDALLControl(_OFF);
		return;
	}
	else if(ucDisplayWaitTimerCnt == 5)
	{
		ucLEDNum = ucNumber[ucDigit];
		if(ucLEDNum == 0)
		{
			LEDALLControl(_ON);
		}
		else
		{
			LEDALLControl(_OFF);
			j = LED_1;

			for(i =0;i<ucLEDNum;i++)
			{
				LEDControl(j,_ON);
				j=j<<1;
			}
			
		}
	}
}

void RunningTimeKeyProcess(unsigned char ucKey)
{
	unsigned char i,k;
	
	if(ucKey == KEY_UP)
	{
		if(ucDigit<5)
		{
			ucDigit++;
			ucDisplayWaitTimerCnt = 7;
		}
		else
		{
			if(ucDigit == 6)
			{
				ucDigit = 0;
				ucDisplayWaitTimerCnt = 7;
			}
			else
			{
			bReturnFlag = _TRUE;
			ucDisplayWaitTimerCnt = 3;
			LEDALLControl(_OFF);
			}
		}			
	}
	else if(ucKey == KEY_DOWN)		
	{
		bTimeAdjustFlag = _TRUE;
		if(ucNumber[ucDigit]>0)
		{
			ucNumber[ucDigit] --;
		}
		else
			ucNumber[ucDigit] = 9;	
		
		ucDisplayWaitTimerCnt = 6;
		bOneSecondFlag = _TRUE;
	}
	else if(ucKey == KEY_UP_DOWN)		
	{		
		if(bTimeAdjustFlag)
		{
			bTimeAdjustFlag = _FALSE;
            k = 5;
			for(i=0;i<6;i++)
			{
				ucRunningHours[k] = ucNumber[i];
                k--;
			}			
			SaveCellRunningTime();
			ucDisplayWaitTimerCnt = 0;
		}
	}
	
}

#endif

void PowerupFunctionModeSelect(void)
{
	KeyScanType tmpkey = KEY_NOTHING;
	#if(RUNNING_TIME_DISPLAY == _ENABLE)
	unsigned char i,k;
	//unsigned int iTemp;
	#endif
	//FAN_CTRL_HIGH;
	
	tmpkey = KeyScanStatus();
	
	if(tmpkey == KEY_UP_DOWN)
	{
		ucFunctionMode = 1; //Model Select Mode
		LEDFlashSetting(0x3C00,_ON);
		ModelChangeLED();
	}
#if(SUPPORT_REVERSE_SETTING == _ENABLE)	
	else if(tmpkey == KEY_DOWN)
	{
		ucFunctionMode = 2; //Reverse Time Setting Mode
	#if(BRAND_SELECT == _AU_CAS_)	
		ucCellReverseSetting = TIME_10HOURS;
		SaveCellReverseSetting();
		LEDFlashSetting(0x0007,_ON);
	#else
		ucCellReverseSetting = TIME_8HOURS;
		SaveCellReverseSetting();
		CellCurrentLED(8);		
	#endif
	}
#endif	
#if(RUNNING_TIME_DISPLAY == _ENABLE)	
	else if(tmpkey == KEY_UP)
	{
		ucFunctionMode = 3; //Running time display
        k = 5;
		for(i=0;i<6;i++)
		{
			ucNumber[i] = ucRunningHours[k];
            k--;
		}
		
#if(SW_VERSION_DISPLAY	==_ENABLE)	
		LEDALLControl(_OFF);
		SWVersionDisplay();
#endif	
		while(KeyScanStatus());
		for(i=0;i<3;i++)
			{
		LEDALLControl(_OFF);
		Delay_ms(100);
		LEDALLControl(_ON);
		Delay_ms(100);
			}
		//bTimeAdjustFlag = _FALSE;
		//bReturnFlag = _FALSE;
		ucDigit = 6;		
	}
#endif	
	else	
	{
		ucFunctionMode = 0; //Normal Mode
	}
	if(ucFunctionMode == 0)	
		ucDisplayWaitTimerCnt = 0;	
	else
		ucDisplayWaitTimerCnt = 11;
	while(1)
	{
		if(ucDisplayWaitTimerCnt == 0)
			break;
		
		KeyDetect();		
		if(g_bKeyDetect)
		{
			if(ucFunctionMode == 1)	
			{
				ModelSelectKeyProcess(g_KeyCode);
			}
#if(SUPPORT_REVERSE_SETTING == _ENABLE)		
			else if(ucFunctionMode == 2)
			{
				ReverseTimeSettingKeyProcess(g_KeyCode);
			}
#endif			
#if(RUNNING_TIME_DISPLAY == _ENABLE)		
			else if(ucFunctionMode == 3)
			{
				RunningTimeKeyProcess(g_KeyCode);
			}
#endif		
			g_bKeyDetect = 0; // release to detect key
		}
		if(bOneSecondFlag)
		{
			bOneSecondFlag = _FALSE;
							
			if(ucDisplayWaitTimerCnt>0)
				ucDisplayWaitTimerCnt--;
			
			bLEDFlashFlag = 1;
			#if(RUNNING_TIME_DISPLAY == _ENABLE)
			RunningLEDDisplay();
			#endif
		}
		LEDFlashDisplay();

	}
	PowerOffLED();
	#if(POWER_BOARD_TEST == _ENABLE)
	ucModelNumber = MODEL_CSM45;
	ucCellCurrent = USER_MAX;
	#else
#if(SUPPORT_REVERSE_SETTING == _ENABLE)		

	if(ucFunctionMode != 2)
	{
#if(SUPPORT_FAST_REVERSE == _ENABLE)	
		if(ucCellReverseSetting == TIME_1HOURS)
		{
			LEDControl(LED_1,_ON);
			Delay_ms(3000);
			LEDControl(LED_1,_OFF);
		}
		else if(ucCellReverseSetting == TIME_2HOURS)
		{
			LEDControl(LED_1,_ON);
			LEDControl(LED_2,_ON);
			Delay_ms(3000);
			LEDControl(LED_1,_OFF);
			LEDControl(LED_2,_OFF);
		}
		else if(ucCellReverseSetting == TIME_8HOURS)
		{
			;
		}
		else
#endif	
		PowerOnLED();
	}
#endif	
	#endif
	ucFunctionMode = 0;
}

void CellReverseForPCBTest(void)
{
	if(bTestModeFlag)
	{
		if(ucCellPolarity)
			ucCellPolarity = 0;
		else 
			ucCellPolarity = 1;	
		bTestModeFlag = 0;
		#if(DEBUG == 0)
			ucPCBTestTimerCnt = 141;
		#else
			ucPCBTestTimerCnt = 0;		
		#endif		
	}
	else
	{
		bTestModeFlag = 1;
		ucPCBTestTimerCnt = 0;
	}	
	#if(DIRECTION_TEST == 1)
		ucPCBTestTimerCnt = 15;
	#endif
	SaveTestModeFlag();

}

void PowerOnInitialize(void)
{
   // unsigned char i;
	//unsigned int j;
	KeyVariableInitial();			//Initial Key Variable 
	ucSecondCounter = 0;
	ucTimer0Counter = 0;
	Delay_ms(200);
	EepromDataCheck();				//Check EEPROM Data
	LEDALLControl(0);

	PowerupFunctionModeSelect();
	CellReverseForPCBTest();
	CellPolarityControl();	
#ifdef USE_DYNAMIC_ZERO_REFERENCE
	GetZeroCurrentReference();
#endif	
	Delay_ms(200);

#if(BURN_IN_TEST == 1) 	
	ucCellCurrent = 50;
	uiOriginalDuty = 500;
#endif	

	CellOutputControl();	
	//bCalibrateMode = 1;
	//Gpio_SetIO(GpioPortA, GpioPin6);

}
#if(WATER_FLOW_CHECK == _ENABLE)
void CheckWaterFlow(void)
{
	if(bProtectFlag == _TRUE)  //Cell Direction Change not check 
		return;
	if(bResetMode)
		return;

	if(WATER_FLOW_SENSOR == 0)
	{
		ucInWaterCounter ++;
	}
	
#if (BURN_IN_TEST == 1)
	ucInWaterCounter = 5;
#endif
}
void WaterFlowFaultProcess(void)
{
	unsigned char bNoWaterFlag;
	
	if(bProtectFlag == _TRUE)  //Cell Direction Change not check 
		return;

	if(ucInWaterCounter == 0)
	{
		bNoWaterFlag = 1;
	}
	else
	{
		bNoWaterFlag = 0;
		ucInWaterCounter = 0;
	}
	
	if(bNoWaterFlag != ucNewWaterFlowStatus)
	{
        ucNewWaterFlowStatus = bNoWaterFlag;				
		bWFProcessFlag = 0;
		ucWaterFlowCounter = 0;	
	}
	else
	{
		ucWaterFlowCounter++;
		if(ucWaterFlowCounter >= 3)
		{
			bWFProcessFlag = 1;
		}
	}

	if(!bWFProcessFlag)
		return;
	if(ucWaterFlowStatus != ucNewWaterFlowStatus)
	{
		eIOTEVENT|=_IOT_EVENT_ERROR;
		ucWaterFlowStatus = ucNewWaterFlowStatus;
//		ucWaterFlowStatus=0;//JIE_DEBUG
		if(ucWaterFlowStatus)
	    {
			ucIOTEVENTMessage=11;
			LEDFlashSetting(LED_12,_ON);	
			CellCurrentOutput(0);
	    }
		else
		{
			ucIOTEVENTMessage=10;
			LEDFlashSetting(LED_12,_OFF);
			LEDControl(LED_12,_OFF);
			CellCurrentOutput(ucCellCurrent);
		}
		
	}
}

#endif
#if(SYSTEM_STATUS_CHECK == _ENABLE)
void CheckCellCurrent(void)
{
	if(uiCellRealCurrent<150)
	{
		if(bLowOutputFlag == _FALSE)
		{
			bLowOutputFlag = _TRUE;
		}
	}
	else
	{
		if(bLowOutputFlag)
		{
			bLowOutputFlag = _FALSE;
		}
	}

}
#endif
#if(CURRENT_CALIBRATION == _ENABLE)
void SpecialSecretCodeProcess(unsigned char ucKey)//[++-++]
{
	if(ucKey == KEY_UP)
	{
		if((bCalibrateMode == 0)&&(bResetMode == 0)&&(ucSecretCode == 0))
		{
			ucCalibrateTimeCnt = 3;
		}
		ucSecretCode|=0x01;
		ucSecretCode =(unsigned char)(ucSecretCode<<1);

	}
	else if(ucKey == KEY_DOWN)
	{
		ucSecretCode = (unsigned char)(ucSecretCode<<1);
	}
	if(ucSecretCode == 0x36)//[+ + - + +]
	{	

		if((bDoBoostModeFlag == 0)&&(bPermanentBoostMode == 0))
		{
			bCalibrateMode = 1;
			ucSecretCode = 0;	
			ucPCBTestTimerCnt = 0;
			ucCellCurrent = USER_MAX;
			LEDFlashSetting(LED_12|LED_11,_ON);
		}
	}
	else if(ucSecretCode == 0x2C)//[+ - + + -]
	{
		bResetMode = 1;
		bPermanentBoostMode = 0;
		bDoBoostModeFlag = 0;
		bCalibrateMode = 0;
		ucSecretCode = 0;
		LEDALLControl(_OFF);
		CellCurrentOutput(0);
		bTimer3EnableFlag = 0;
		LEDFlashSetting((LED_1|LED_10),_ON);
	}
}
#endif

#if(CURRENT_CALIBRATION == _ENABLE)
void CalibrateParameter(void)
{
	unsigned int ulTemp;
	unsigned int uiTemp;
	ulTemp = 80566;
	ulTemp = ulTemp*uiCurrentADC;
	uiTemp = ulTemp/uiLimitCurrent;
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)	
	uiTemp += 500;
	ucGainValue =(unsigned char)(uiTemp/1000);
#else
	uiTemp += 50;
	ucGainValue = uiTemp/100;

#endif
}
#endif

void ChlorBoostProcess(void)
{
#if(CURRENT_CALIBRATION == _ENABLE)    
	if(bCalibrateMode)
	{
		ucCurrentCalValue = ucGainValue;
		CalculateConvertValue();
		SaveCalValueByDirection();
		bCalibrateMode = 0;
		ucSecretCode = 0;
		uiCurrentDuty = uiPWMHCnt;
		LEDFlashSetting(LED_12,_OFF);
		if(ucCellPolarity ==_FORWARD)
		{
			LEDFlashSetting(LED_11,_OFF);
			LEDControl(LED_11, _ON);
		}
		return;
	}
#endif    
#if(BRAND_SELECT == _AU_CAS_)
	if(bPermanentBoostMode)
		return;
	if(ucBoostStatus == 0)
	{
		PowerOffLED();
		LEDControl(LED_10, _ON);
		ucDisplayWaitTimerCnt = 2;
		ucBoostStatus++;	
	}
	else if(ucBoostStatus == 2)
	{
		LEDControl(LED_9, _OFF);
		LEDControl(LED_8, _ON);
		ucDisplayWaitTimerCnt = 2;			
		ucBoostStatus++;				
	}
	else if(ucBoostStatus == 4)
	{
		LEDControl(LED_7, _OFF);
		LEDControl(LED_6, _ON);
		ucDisplayWaitTimerCnt = 2;	
		ucBoostStatus++;	
	}
#endif
}

void ComboKeyProcess(void)
{
	if(bResetMode)
	{
		FactoryReset();
		LEDControl(LED_12,_ON);
		Delay_ms(100);
		LEDControl(LED_12,_OFF);
		Delay_ms(100);
		LEDControl(LED_12,_ON);
		Delay_ms(100);
		LEDControl(LED_12,_OFF);
		Delay_ms(100);
		LEDControl(LED_12,_ON);
		Delay_ms(100);
		LEDControl(LED_12,_OFF);
		Delay_ms(100);
		LEDFlashSetting(LED_12,_ON);
		return;	
	}	
	ChlorBoostProcess();
}

void BoostModeHandler(void)
{
	if(bPermanentBoostMode)
		return;

	if(ucDisplayWaitTimerCnt == 0)
	{
		switch(ucBoostStatus)
		{
			case 1:
				LEDControl(LED_10,_OFF);
				LEDControl(LED_9,_ON);
				ucBoostStatus++;
				ucDisplayWaitTimerCnt = 2;
				break;
				
			case 2:
				LEDControl(LED_9,_OFF);		
				LEDControl(LED_11,_ON);
				if(ucCellPolarity == _REVERSE)
				LEDFlashSetting(LED_11,_ON);
				LEDFlashSetting(0x3FF,_ON);
				ucBoostStatus = 0;
				bDoBoostModeFlag = 1;
				uiBoostCounter = 0;
				GetModelRatingCurrent();
				CalculateThresholdLimit();
				CellCurrentOutput(USER_MAX);					
				break;
				
			case 3:
				LEDControl(LED_8,_OFF);
				LEDControl(LED_7,_ON);
				ucBoostStatus++;
				ucDisplayWaitTimerCnt = 2;	
				break;
			case 4:
				LEDControl(LED_7,_OFF);
				LEDControl(LED_11,_ON);
				if(ucCellPolarity == _REVERSE)
				LEDFlashSetting(LED_11,_ON);
				ucBoostStatus = 0;
				bDoBoostModeFlag = 0;
				GetModelRatingCurrent();
				CalculateThresholdLimit();
				CellOutputCurrentAdjust(0);				
				break;
			case 5:
				LEDControl(LED_6,_OFF);
				LEDControl(LED_5,_ON);
				ucBoostStatus++;
				ucDisplayWaitTimerCnt = 2;
				ucPermanentBoostCnt++;
				if(ucPermanentBoostCnt == 3)
				{
					bPermanentBoostMode = 1;
					ucPermanentBoostCnt = 0;
				}
				break;
			case 6:
				LEDControl(LED_5,_OFF);
				LEDControl(LED_11,_ON);
				if(ucCellPolarity == _REVERSE)
				LEDFlashSetting(LED_11,_ON);
				ucBoostStatus = 0;
				bDoBoostModeFlag = 0;
				break;
			default:
				ucBoostStatus = 0;
				break;
		}
	}

}

void SaveEventProcess(void)
{
	if(ucSaveType == _NOTHING)
		return;
	if(ucSaveType & _USER_SETTING)
	{
		SaveDataToArea(EEPROM_START_ADDRESS,13,&ucData[0]);
		ucSaveType &= ~_USER_SETTING;
		ucSaveType |= _FACTORY_DATA;
	}
	if(ucSaveType & _RUNNING_DATA)
	{
		SaveDataToArea(CELL_RUNNING_MINS_ADDR,7,&ucData[13]);
		ucSaveType &= ~_RUNNING_DATA;
	}
	if(ucSaveType & _FACTORY_DATA)
	{
		SaveDataToArea(DATA_BACKUP_ADDR,13,&ucData[0]);
		ucSaveType &= ~_FACTORY_DATA;
	}
}

#if(AC_POWER_CHECK == _ENABLE)
#if(BOARD_VERSION == BOARD_EVT_VERSION) 
#define AC_POWER_DETECT   Gpio_GetInputIO(GpioPortA, GpioPin12)
#else
#define AC_POWER_DETECT   Gpio_GetInputIO(GpioPortB, GpioPin13)
#endif
void CheckACPowerStatus(void)
{
	if(AC_POWER_DETECT == 1)
		ucACPowerCounter++;
}
void ACPowerStatusProcess(void)
{
	unsigned char ucTempStatus;

	if(ucACPowerCounter <= 3)
		ucTempStatus = 1;
	else
		ucTempStatus = 0;

	if(ucTempStatus != bACPowerOffFlag)
	{
		if(ucTempStatus)
		{
			ucACOffCounter++;
			if(ucACOffCounter >= 2)	
			{
				MCUStopOutput();
				bACPowerOffFlag = ucTempStatus;
			}
			ucACOnCounter = 0;
			
		}
		else
		{
			ucACOnCounter++;
			if(ucACOnCounter >= 2)
			{
				#if(SUPPORT_1KHZ == _ENABLE)
				uiPWMHCnt = 5;
				uiPWMLCnt = 995;
				#else
				uiPWMHCnt = 50;
				uiPWMLCnt = 9950;
				#endif
				PowerOnInitialize();
				bACPowerOffFlag = ucTempStatus;
			}
			ucACOffCounter = 0;
		}
	}
	ucACPowerCounter = 0;
}
#endif

#if(LOW_SALT_CHECK == _ENABLE)
void CheckWaterSaltLevel(void)
{
	unsigned int uiLowSaltCurrent;

	uiLowSaltCurrent = uiStdL/2;
	
	if((uiCurrentDuty >= uiCurrentMaxDuty)&&(uiCellRealCurrent < uiLowSaltCurrent)&&(bStopOutputFlag == _FALSE))
	{
		if(bLowOutputFlag == _FALSE)
		{
			bLowOutputFlag = _TRUE;
			LEDFlashSetting(LED_12,_ON);
			eIOTEVENT|=_IOT_EVENT_ERROR;
			ucIOTEVENTMessage=11;
		}
	}
	else
	{
		if(bLowOutputFlag == _TRUE)
		{
			bLowOutputFlag = _FALSE;
			if(ucWaterFlowStatus == 0)
			{
				LEDFlashSetting(LED_12,_OFF);
				LEDControl(LED_12,_OFF);
				eIOTEVENT|=_IOT_EVENT_ERROR;
				ucIOTEVENTMessage=10;
			}
		}
	}
}
#endif

#if(BRAND_SELECT == _EU_DAVEY_)	
void CellCurrentCycleOutput(void)
{		
	if((bOutputFlag == _ENABLE)&&(ucCellCurrent < USER_MAX))
	{
		uiOutputCounter++;
		if(uiOutputCounter == OUTPUT_PERIOD)
		{
			uiOutputCounter = 0;
			bTimer3EnableFlag = _TRUE;
			PWMStart();
			bStopOutputFlag = _FALSE;
		}
		else if(uiOutputCounter == uiOutputTime)
		{		
			bStopOutputFlag = _TRUE;
			PWMStop();
			uiCurrentDuty = PWM_MIN;
			uiPWMHCnt = PWM_CNT_MIN;
			CURRENT_CTRL_OFF;				
			bTimer3EnableFlag = _FALSE;
		}
	}

}
#endif			

void ProcessHandler(void)
{	
#if(WATER_FLOW_CHECK == _ENABLE)
	CheckWaterFlow();		//Water Flow Sensor
#endif
#if(AC_POWER_CHECK == _ENABLE)
	do
	{
		CheckACPowerStatus();
		if(bOneSecondFlag)
		{
			ACPowerStatusProcess();
			if(bACPowerOffFlag)			
				bOneSecondFlag = _FALSE;
		}		
	}
	while (bACPowerOffFlag);
#endif	
	
	if(bOneSecondFlag)
	{
//		UartSendString("oneseconds\n");
		bOneSecondFlag = _FALSE;

		if(ucPCBTestTimerCnt>0)
			ucPCBTestTimerCnt--;
						
		if(ucDisplayWaitTimerCnt>0)
			ucDisplayWaitTimerCnt--;

		bLEDFlashFlag = 1;
		
#if(CURRENT_CALIBRATION == _ENABLE) 		
		if(ucCalibrateTimeCnt>0)
		{
			ucCalibrateTimeCnt--;
			if(ucCalibrateTimeCnt == 0)
			{
				ucSecretCode = 0;
			}
		}	
		if(bCalibrateMode)
		{
			CalibrateParameter();
		}
#endif	

		
		#if(WATER_FLOW_CHECK == _ENABLE)	 	
		WaterFlowFaultProcess();
		#endif

		#if(DIRECTION_TEST == 1)
		if(ucPCBTestTimerCnt == 6)
		{
			CellCurrentLED(0);
			ucCellCurrent = USER_MIN;
			CURRENT_CTRL_OFF;
			ucDisplayWaitTimerCnt = 3;
		}
		else if(ucPCBTestTimerCnt == 5)
		{
			if(ucCellPolarity)
				ucCellPolarity = 0;
			else
				ucCellPolarity = 1;
			CellPolarityControl();
		}
		else if(ucPCBTestTimerCnt == 1)	
		{
			CellCurrentLED(10);
			ucCellCurrent = USER_MAX;
			CURRENT_CTRL_ON;
			ucPCBTestTimerCnt = 15;
			ucDisplayWaitTimerCnt = 3;
		}
			
		#else
		if(ucPCBTestTimerCnt == 111)//Over 30s, clear test mode flags
		{
			bTestModeFlag = 0;
			SaveTestModeFlag();
		}
		else if(ucPCBTestTimerCnt == 21)
		{
			#if(BURN_IN_TEST == 1)
			uiOriginalDuty = uiCurrentDuty; 			
			#endif
			CellCurrentLED(0);
			CellCurrentOutput(USER_MIN);
			bProtectFlag = _TRUE;
		}
		else if(ucPCBTestTimerCnt == 12)
		{
			ReadCellPolarity();
			CellPolarityControl();
			#if(BRAND_SELECT == _AU_CAS_)
			ucOriginalCurrent = ucCellCurrent;
			ucCellCurrent = USER_MIN;	
			#endif
			bProtectFlag = _FALSE;
		}
		else if((ucPCBTestTimerCnt>=1)&&(ucPCBTestTimerCnt<=11))
		{	
		#if(BRAND_SELECT == _AU_CAS_)	
			if(ucCellCurrent<ucOriginalCurrent)
				ucCellCurrent+=10;
			CellOutputCurrentAdjust(0);
			
			#if(BURN_IN_TEST == 1)
			ucDisplayWaitTimerCnt = 0;
			#else
			ucDisplayWaitTimerCnt = 3;
			#endif
			
			if(ucPCBTestTimerCnt == 1)//Over 120s, Set back the cell polarity
			{			
				ucDisplayWaitTimerCnt = 0;
			}
		#else
		if(ucPCBTestTimerCnt == 11)
		{
			uiOutputTime = 6*ucCellCurrent;
			CellCurrentOutput(ucCellCurrent);	
		}
		#endif
		}
		#endif
		iSaveCounter++;
		if(iSaveCounter == 300)//Save the reverse time every 5 minutes	
		{
			ucCellReverseTime++;
			#if(BURN_IN_TEST == 1)
			if(ucCellReverseTime >=6)// 30 MINUTES
			#else
			if(ucCellReverseTime >=ucCellReverseSetting)
			#endif
			{
				ucCellReverseTime = 0;
				CellPolarityReverse();
	#ifdef USE_DYNAMIC_ZERO_REFERENCE
				GetZeroCurrentReference();
	#endif
				CellCurrentLED(0);
				CellCurrentOutput(USER_MIN);
				bProtectFlag = _TRUE;
				ucPCBTestTimerCnt = 71;
			}
			iSaveCounter = 0;
			SaveCellReverseTime();
#if(RUNNING_TIME_DISPLAY == _ENABLE)
			ucRunningMins+=5;
			if(ucRunningMins >= 60)
			{	
				unsigned char i;
				ucRunningMins = 0;
				for(i=5;i>0;i--)
				{
					ucRunningHours[i]++;
					if(ucRunningHours[i] ==10)
					{
						ucRunningHours[i] = 0;
					}
					else
					{
						break;
					}
				}
				SaveCellRunningTime();
			}
			SaveCellRunningMinutes();
					
#endif
			
		}
		
		if(bDoBoostModeFlag)
		{
			uiBoostCounter++;
			if(uiBoostCounter>=28800)
			{
				bDoBoostModeFlag = 0;
				GetModelRatingCurrent();
				CalculateThresholdLimit();
				CellOutputCurrentAdjust(0);
			}
		}
		BoostModeHandler();
		if((ucBoostStatus==0)&&(bDoBoostModeFlag==0)&&(bResetMode==0))
		{
			if(ucDisplayWaitTimerCnt == 0)
			{
				CellRealCurrentDisplay();	
				#if(SYSTEM_STATUS_CHECK == _ENABLE)
				if(bProtectFlag == _FALSE)
				CheckCellCurrent();
				#endif
			}		
		}
		
	SaveEventProcess();
	}
	if(ucBoostStatus==0)
	{
		GetCellRealCurrent();
#if(LOW_SALT_CHECK == _ENABLE)
		CheckWaterSaltLevel();
#endif
		if(bADCUpdateFlag)
		{
			bCalibrationFlag = _FALSE;
			CellOutputCalibration();
			bADCUpdateFlag = _FALSE;
		}
	
	}
	LEDFlashDisplay();	

}

void CurrentADCProcess(void)
{
	ucADCCounter++;
	iADCSum += u16AdcRestult0;
	if(ucADCCounter >= ADCMAXCNT)
	{
		ADCStop();
		StopTimer1();	
		//Gpio_WriteOutputIO(GpioPortA, GpioPin6,FALSE);
		if(bADCDoneFlag == _FALSE)
		{
			bADCDoneFlag = _TRUE;
			iADCData = iADCSum;
		}
		iADCSum = 0;
		ucADCCounter = 0;	
	}

}

void MainTimerHandler(void)
{
	ucTimer0Counter++;
	APP_UART_1MS_HANDLE();
	if(ucTimer0Counter>=100)
	{		
		ucTimer0Counter = 0;
		ucSecondCounter++;
		if(ucSecondCounter==5)
		{
			bCalibrationFlag = _TRUE;
			//CellCurrentCycleOutput();
		}
		else if(ucSecondCounter>=10)
		{
			ucSecondCounter = 0;
			bOneSecondFlag = _TRUE;
			bCalibrationFlag = _TRUE;
			CellCurrentCycleOutput();
		}
		
	}	

}
