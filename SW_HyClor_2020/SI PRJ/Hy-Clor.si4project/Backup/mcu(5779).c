/******************************************************************************
 * mcu.c 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     mcu setting
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#include "gpio.h"
#include "bt.h"
#include "sysctrl.h"
#include "flash.h"
#include "adc.h"
#include "timer3.h"
#include "global.h"
#include "keypad.h"
#include "ctrl.h"

//uint16_t u16AdcRestult0;
unsigned short int u16AdcRestult0 = 0;
unsigned char ucTestCnt =0;

static stc_adc_irq_t stcAdcIrqFlag;

void PWMStart(void)
{
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)
    EnableNvic(TIM3_IRQn, IrqLevel0, TRUE);                   //TIM3中断使能
    
    Tim3_M23_EnPWM_Output(TRUE, FALSE);                       //端口输出使能

	Tim3_M23_Run();
#else
	Tim3_ClearIntFlag(Tim3UevIrq);
    Tim3_Mode0_EnableIrq();
    EnableNvic(TIM3_IRQn, IrqLevel0, TRUE);                   //TIM3中断使能
    EnableNvic(PORTA_IRQn, IrqLevel3, TRUE);  
#endif
}

void PWMStop(void)
{
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)
    EnableNvic(TIM3_IRQn, IrqLevel0, FALSE);                   //TIM3中断使能
    
    Tim3_M23_EnPWM_Output(FALSE, FALSE);                       //端口输出使能

	Tim3_M23_Stop();
#else
	Tim3_ClearIntFlag(Tim3UevIrq);
	Tim3_Mode0_DisableIrq();
	EnableNvic(TIM3_IRQn, IrqLevel0, FALSE);                   //TIM3中断使能
	Tim3_M0_Stop();
	Gpio_ClearIrq(GpioPortA, GpioPin12);
	EnableNvic(PORTA_IRQn, IrqLevel3, FALSE); 
#endif
}


void ALLStop(void)
{
	Adc_SGL_Stop();
	Bt_M0_Stop(TIM1);
	Bt_M0_Stop(TIM0);
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)	
	EnableNvic(TIM3_IRQn, IrqLevel0, _FALSE);
#else
	PWMStop();
	Gpio_SetIO(GpioPortC, GpioPin13);
#endif	
}

void ALLStart(void)
{
	//Adc_SGL_Stop();
	Bt_M0_Run(TIM1);
	Bt_M0_Run(TIM0);
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)	
	EnableNvic(TIM3_IRQn, IrqLevel0, _TRUE);
#else
	PWMStart(); 
	Gpio_SetIO(GpioPortC, GpioPin13);
	EnableNvic(PORTA_IRQn, IrqLevel3, TRUE); 
#endif
}

void ADCStart(void)
{
	//while(FALSE == stcAdcIrqFlag.bAdcIrq);
	stcAdcIrqFlag.bAdcIrq = FALSE;	
	Adc_SGL_Start();
}

void ADCStop(void)
{
	Adc_SGL_Stop();
}

void StartTimer1(void)
{
	//Bt_M0_ARRSet(TIM1, 65501);

    //Bt_M0_Cnt16Set(TIM1, 65506);                      //设置计数初值
    
    //Bt_ClearIntFlag(TIM1,BtUevIrq);                         //清中断标志   

	//Bt_Mode0_EnableIrq(TIM1);								//使能TIM1中断(模式0时只有一个中断)
	//EnableNvic(TIM1_IRQn, IrqLevel2, TRUE); 				//TIM1中断使能
	Bt_M0_Run(TIM1);

}

void StopTimer1(void)
{
	Bt_M0_Stop(TIM1);
	//EnableNvic(TIM1_IRQn, IrqLevel2, FALSE); 
	//Bt_Mode0_DisableIrq(TIM1);
	//Gpio_WriteOutputIO(GpioPortA, GpioPin6,FALSE);
}

void AdcContIrqCallback(void)
{    
    Adc_GetSglResult(&u16AdcRestult0);
    stcAdcIrqFlag.bAdcIrq = TRUE;
	CurrentADCProcess();	
}


void Gpio_IRQHandler(uint8_t u8Param)
{
	unsigned int uiTemp;
#if(AC_POWER_CHECK == _ENABLE)  
	#if(BOARD_VERSION == BOARD_EVT_VERSION) 
    ///< PORT A
    if(0 == u8Param)
    {
        if(TRUE == Gpio_GetIrqStatus(GpioPortA, GpioPin12))
        {
            Gpio_ClearIrq(GpioPortA, GpioPin12);
        }

    }
	#else
	///< PORT B
	if(1 == u8Param)
	{
        if(TRUE == Gpio_GetIrqStatus(GpioPortB, GpioPin13))
        {
            Gpio_ClearIrq(GpioPortB, GpioPin13);
                   
        }

    }	
	#endif
#else
    ///< PORT A
    if(0 == u8Param)
    {
        if(TRUE == Gpio_GetIrqStatus(GpioPortA, GpioPin12))
        {
            Gpio_ClearIrq(GpioPortA, GpioPin12);
            /*if(Gpio_GetInputIO(GpioPortA, GpioPin6))    
				Gpio_ClrIO(GpioPortA, GpioPin6);
			else
				Gpio_SetIO(GpioPortA, GpioPin6);*/   
			//CURRENT_CTRL_OFF;
			Gpio_SetIO(GpioPortC, GpioPin13);
			if(bStopOutputFlag == _FALSE)
			{
				//uiTemp = 0xFFFF- uiPWMPeriod;
				uiTemp = 55535 + uiPWMHCnt;
				Tim3_M0_Cnt16Set(uiTemp);
				Tim3_M0_ARRSet(uiTemp);
				Tim3_M0_Run();
			}
	    	StartTimer1();
			ADCStart();	
			//Gpio_WriteOutputIO(GpioPortA, GpioPin6,TRUE);
        }
		
    }
#endif	
} 

/*******************************************************************************
 * BT1中断服务函数
 ******************************************************************************/
void Tim0Int(void)
{
    
    if(TRUE == Bt_GetIntFlag(TIM0, BtUevIrq))
    {
		KeyDetectTimer();
		MainTimerHandler();
        Bt_ClearIntFlag(TIM0,BtUevIrq);
    }
}
void Tim1Int(void)
{
    if(TRUE == Bt_GetIntFlag(TIM1, BtUevIrq))
    {
		//Bt_M0_Cnt16Set(TIM1, 65486);
		ADCStart();
		//ucTestCnt++;
		//if(ucTestCnt ==10)
		//{
		//	StopTimer1();
		//	ucTestCnt =0;
		//}
		Bt_ClearIntFlag(TIM1,BtUevIrq);
            //if(Gpio_GetInputIO(GpioPortA, GpioPin6))    
			//	Gpio_ClrIO(GpioPortA, GpioPin6);
			//else
			//	Gpio_SetIO(GpioPortA, GpioPin6);			
    }
}

void Tim3Int(void)
{
   // static unsigned char i;
    
    if(TRUE == Tim3_GetIntFlag(Tim3UevIrq))
    {
    #if(POWER_BOARD_VERSION == POWER_MOS_VERSION)
    	StartTimer1();
		ADCStart();	
		//Gpio_WriteOutputIO(GpioPortA, GpioPin6,TRUE);
        if(0 == i)
        {
            //Gpio_WriteOutputIO(GpioPortB, GpioPin0,TRUE);                       
            Tim3_M23_CCR_Set(Tim3CCR1A, uiPWMHCnt);
            Tim3_M23_CCR_Set(Tim3CCR1B, uiPWMLCnt);                      
            i++;
        }
        else if(1 == i)
        {
            //Gpio_WriteOutputIO(GpioPortB, GpioPin0,FALSE);                                                
            i = 0;
        }
	#else
		//if(CURRENT_CTRL == _HIGH)
		if(Gpio_ReadOutputIO(GpioPortC, GpioPin13) == _HIGH)
		{
			//CURRENT_CTRL_ON;
			Gpio_ClrIO(GpioPortC, GpioPin13);
			Tim3_M0_Cnt16Set(65036);
			Tim3_M0_ARRSet(65036);			
		}
		else
		{
			//CURRENT_CTRL_OFF;
			Gpio_SetIO(GpioPortC, GpioPin13);
			Tim3_M0_Cnt16Set(55536);
			Tim3_M0_ARRSet(55536);					
		}
	#endif	
		
        Tim3_ClearIntFlag(Tim3UevIrq);
    }
}

void Init_OscFrequency(void)
{
    stc_sysctrl_clk_config_t stcCfg;
	
    ///<========================== 时钟初始化配置 ===================================
    ///< 因要使用的时钟源HCLK小于24M：此处设置FLASH 读等待周期为0 cycle(默认值也为0 cycle)
    Flash_WaitCycle(FlashWaitCycle0);
    
    ///< 时钟初始化前，优先设置要使用的时钟源：此处设置RCH为4MHz
    Sysctrl_SetRCHTrim(SysctrlRchFreq16MHz);
    
    ///< 选择内部RCH作为HCLK时钟源;
    stcCfg.enClkSrc    = SysctrlClkRCH;
    ///< HCLK SYSCLK/1
    stcCfg.enHClkDiv   = SysctrlHclkDiv1;
    ///< PCLK 为HCLK/1
    stcCfg.enPClkDiv   = SysctrlPclkDiv1;
    ///< 系统时钟初始化
    Sysctrl_ClkInit(&stcCfg);
	
}

void Init_IOPorts(void)
{
    stc_gpio_config_t stcGpioCfg;

    DDL_ZERO_STRUCT(stcGpioCfg);
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
    ///< 端口方向配置->输入
    stcGpioCfg.enDir = GpioDirIn;
    ///< 端口驱动能力配置->高驱动能力
    stcGpioCfg.enDrv = GpioDrvL;
    ///< 端口上下拉配置->上拉
    stcGpioCfg.enPuPd = GpioPu;
    ///< 端口开漏输出配置->开漏输出关闭
    stcGpioCfg.enOD = GpioOdDisable;
    ///< 端口输入/输出值寄存器总线控制模式配置->AHB
    stcGpioCfg.enCtrlMode = GpioAHB;
	
    //Input Singal
#if(BOARD_SELECT == BOARD_LED_BUTTON)
    Gpio_Init(GpioPortA, GpioPin4, &stcGpioCfg);   //S1
    Gpio_Init(GpioPortA, GpioPin5, &stcGpioCfg);   //S2
#else
    Gpio_Init(GpioPortA, GpioPin4, &stcGpioCfg);   //S1
    Gpio_Init(GpioPortA, GpioPin5, &stcGpioCfg);   //S2
#endif
#if(WATER_SENSOR_TYPE == EXT_SENSOR)
    Gpio_Init(GpioPortA, GpioPin0, &stcGpioCfg);   //Water Flow
#else
    Gpio_Init(GpioPortA, GpioPin1, &stcGpioCfg);   //Water Flow
#endif 
#if(AC_POWER_CHECK == _ENABLE)  
    ///< 端口上下拉配置->上拉
    stcGpioCfg.enPuPd = GpioNoPuPd;

	#if(BOARD_VERSION == BOARD_EVT_VERSION)    
	Gpio_Init(GpioPortA, GpioPin12, &stcGpioCfg);   //AC POWER
    
    Gpio_ClearIrq(GpioPortA, GpioPin12);
    ///< 打开并配置PA12为下降沿中断
    Gpio_EnableIrq(GpioPortA, GpioPin12, GpioIrqFalling);
    ///< 使能端口PORTA系统中断
    EnableNvic(PORTA_IRQn, IrqLevel3, TRUE);	
	#else
	Gpio_Init(GpioPortB, GpioPin13, &stcGpioCfg);   //AC POWER
    Gpio_ClearIrq(GpioPortB, GpioPin13);
    ///< 打开并配置PB13为下降沿中断
    Gpio_EnableIrq(GpioPortB, GpioPin13, GpioIrqFalling);
    ///< 使能端口PORTB系统中断
    EnableNvic(PORTB_IRQn, IrqLevel3, TRUE);	
	#endif
#else
    Gpio_Init(GpioPortA, GpioPin12, &stcGpioCfg);  //Zero Cross Detect
    Gpio_ClearIrq(GpioPortA, GpioPin12);
    ///< 打开并配置PA12为下降沿中断
    Gpio_EnableIrq(GpioPortA, GpioPin12, GpioIrqFalling);
    ///< 使能端口PORTA系统中断
    //EnableNvic(PORTA_IRQn, IrqLevel3, TRUE);   
#endif
    ///< 端口方向配置->输出
    stcGpioCfg.enDir = GpioDirOut;
    ///< 端口驱动能力配置->高驱动能力
    stcGpioCfg.enDrv = GpioDrvH;
    ///< 端口上下拉配置->下拉
    stcGpioCfg.enPuPd = GpioNoPuPd;	

	//Output Control 
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)	
	Gpio_ClrIO(GpioPortC, GpioPin13);
	Gpio_Init(GpioPortC, GpioPin13, &stcGpioCfg);	
	Gpio_SetIO(GpioPortC, GpioPin13);
#else
	Gpio_SetIO(GpioPortC, GpioPin13);
	Gpio_Init(GpioPortC, GpioPin13, &stcGpioCfg);	
#endif
	
	//Direction Control	
	Gpio_ClrIO(GpioPortD, GpioPin0);
	Gpio_Init(GpioPortD, GpioPin0, &stcGpioCfg);	
	Gpio_ClrIO(GpioPortD, GpioPin1);
	Gpio_Init(GpioPortD, GpioPin1, &stcGpioCfg);	
	
#if(BOARD_SELECT == BOARD_LED_BUTTON)
	///< 端口上下拉配置->无上下拉
    stcGpioCfg.enPuPd = GpioNoPuPd;
	//LED D1--> D12
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
	
    Gpio_Init(GpioPortA, GpioPin8, &stcGpioCfg);
	Gpio_Init(GpioPortB, GpioPin8, &stcGpioCfg);
#if(BOARD_VERSION == BOARD_EVT_VERSION)	
    Gpio_Init(GpioPortA, GpioPin10, &stcGpioCfg);
	Gpio_Init(GpioPortB, GpioPin13, &stcGpioCfg);
#else
    Gpio_Init(GpioPortB, GpioPin7, &stcGpioCfg);
	Gpio_Init(GpioPortA, GpioPin9, &stcGpioCfg);
#endif
    Gpio_Init(GpioPortB, GpioPin9, &stcGpioCfg);
	Gpio_Init(GpioPortB, GpioPin11, &stcGpioCfg);
    Gpio_Init(GpioPortB, GpioPin10, &stcGpioCfg);
	Gpio_Init(GpioPortB, GpioPin2, &stcGpioCfg);
    Gpio_Init(GpioPortB, GpioPin1, &stcGpioCfg);
	Gpio_Init(GpioPortB, GpioPin0, &stcGpioCfg);
    Gpio_Init(GpioPortA, GpioPin7, &stcGpioCfg);
	Gpio_Init(GpioPortA, GpioPin6, &stcGpioCfg);
	

#else
	//LCD control I/O
    Gpio_Init(GpioPortA, GpioPin10, &stcGpioCfg);	//RESET	
	Gpio_Init(GpioPortB, GpioPin12, &stcGpioCfg);   //CS
    Gpio_Init(GpioPortB, GpioPin13, &stcGpioCfg);   //RS
	Gpio_Init(GpioPortA, GpioPin8, &stcGpioCfg);    //SCL
    Gpio_Init(GpioPortA, GpioPin15, &stcGpioCfg);   //SI
	Gpio_SetIO(GpioPortA, GpioPin10);               //RESET
	Gpio_SetIO(GpioPortB, GpioPin12);          
	Gpio_ClrIO(GpioPortB, GpioPin13);
	Gpio_ClrIO(GpioPortA, GpioPin8);
	Gpio_ClrIO(GpioPortA, GpioPin15);
#endif

}

void Init_Timer(void)
{
    uint16_t                  u16ArrValue;
    uint16_t                  u16CntValue;
    stc_bt_mode0_config_t     stcBtBaseCfg;
    
    stc_bt_mode0_config_t     stcBtTimer1Cfg;

#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
	
    stc_tim3_mode0_config_t        stcTim3BaseCfg;
	DDL_ZERO_STRUCT(stcTim3BaseCfg);
	
#endif
    DDL_ZERO_STRUCT(stcBtBaseCfg);
    
    DDL_ZERO_STRUCT(stcBtTimer1Cfg);
	
    Sysctrl_SetPeripheralGate(SysctrlPeripheralBTim, TRUE); //Base Timer外设时钟使能
    
    
    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //定时器模式
    stcBtBaseCfg.enCT       = BtTimer;                      //定时器功能，计数时钟为内部PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv4;//BtPCLKDiv64;                  //PCLK/64
    stcBtBaseCfg.enCntMode  = Bt16bitArrMode;               //自动重载16位计数器/定时器
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    
    stcBtBaseCfg.pfnTim0Cb  = Tim0Int;                      //中断函数入口
    
    Bt_Mode0_Init(TIM0, &stcBtBaseCfg);                     //TIM0 的模式0功能初始化
    
    u16ArrValue = 0x10000-1000*4;//64536;////0x6000;
    
    Bt_M0_ARRSet(TIM0, u16ArrValue);                        //设置重载值(周期 = 0x10000 - ARR)
    
    u16CntValue = 0x10000-1000*4;///64536;//0x6000;
    
    Bt_M0_Cnt16Set(TIM0, u16CntValue);                      //设置计数初值
    
    Bt_ClearIntFlag(TIM0,BtUevIrq);                         //清中断标志   
    Bt_Mode0_EnableIrq(TIM0);                               //使能TIM0中断(模式0时只有一个中断)
    EnableNvic(TIM0_IRQn, IrqLevel3, TRUE);                 //TIM0中断使能
    
    Bt_M0_Run(TIM0);                                        //TIM0 运行。

//Timer 1
    stcBtTimer1Cfg.enWorkMode = BtWorkMode0;                  //定时器模式
    stcBtTimer1Cfg.enCT       = BtTimer;                      //定时器功能，计数时钟为内部PCLK
    stcBtTimer1Cfg.enPRS      = BtPCLKDiv4;//BtPCLKDiv4//BtPCLKDiv64;                  //PCLK/64
    stcBtTimer1Cfg.enCntMode  = Bt16bitArrMode;               //自动重载16位计数器/定时器
    stcBtTimer1Cfg.bEnTog     = FALSE;
    stcBtTimer1Cfg.bEnGate    = FALSE;
    stcBtTimer1Cfg.enGateP    = BtGatePositive;
    
    stcBtTimer1Cfg.pfnTim1Cb  = Tim1Int;                      //中断函数入口
    
    Bt_Mode0_Init(TIM1, &stcBtTimer1Cfg);                     //TIM0 的模式0功能初始化
 #if(POWER_BOARD_VERSION == POWER_TRC_VERSION)   
    u16ArrValue =0x10000- (0x10000-65411)*2;
//  u16ArrValue = 65411;
 #else
    u16ArrValue =0x10000- (0x10000-65436)*2;
//    u16ArrValue = 65436;
 #endif
    Bt_M0_ARRSet(TIM1, u16ArrValue);                        //设置重载值(周期 = 0x10000 - ARR)
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)     
    u16ArrValue =0x10000- (0x10000-65411)*2;
//  u16CntValue = 65411;
#else
    u16ArrValue =0x10000- (0x10000-65436)*2;
//	u16CntValue = 65436;
#endif
    Bt_M0_Cnt16Set(TIM1, u16CntValue);                      //设置计数初值
    
    Bt_ClearIntFlag(TIM1,BtUevIrq);                         //清中断标志   
    Bt_Mode0_EnableIrq(TIM1);                               //使能TIM1中断(模式0时只有一个中断)
    EnableNvic(TIM1_IRQn, IrqLevel3, TRUE);                 //TIM1中断使能
	//Bt_M0_Run(TIM1);

#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
	Sysctrl_SetPeripheralGate(SysctrlPeripheralTim3, TRUE);   //Timer3外设时钟使能

	stcTim3BaseCfg.enWorkMode    = Tim3WorkMode0;             //定时器模式
    stcTim3BaseCfg.enCT          = Tim3Timer;                 //定时器功能，计数时钟为内部PCLK
    stcTim3BaseCfg.enPRS         = Tim3PCLKDiv4;              //PCLK
    stcTim3BaseCfg.enCntMode     = Tim316bitArrMode;               //自动重载16位计数器/定时器
    stcTim3BaseCfg.bEnTog        = FALSE;
    stcTim3BaseCfg.bEnGate       = FALSE;
    stcTim3BaseCfg.enGateP       = Tim3GatePositive;

	stcTim3BaseCfg.pfnTim3Cb  = Tim3Int;                      //中断函数入口
	Tim3_Mode0_Init(&stcTim3BaseCfg);                        //TIM3 的模式0功能初始化

    u16ArrValue = 0xFFFF-(0xFFFF-uiPWMPeriod)*4;
	Tim3_M0_Cnt16Set(u16ArrValue);
    Tim3_M0_ARRSet(u16ArrValue);                       //设置重载值,并使能缓存	
    Tim3_ClearIntFlag(Tim3UevIrq);
    //Tim3_Mode0_EnableIrq();
	//EnableNvic(TIM3_IRQn, IrqLevel0, TRUE);
	//Tim3_M0_Run();
#endif
}

void Init_ADC(void)
{
    //uint8_t                    u8AdcScanCnt;
    //uint32_t                   u32AdcResultAcc;
    stc_adc_cfg_t              stcAdcCfg;
    stc_adc_irq_t              stcAdcIrq;
    stc_adc_irq_calbakfn_pt_t  stcAdcIrqCalbaks;
    stc_gpio_config_t          stcAdcAN0Port;    
    
    DDL_ZERO_STRUCT(stcAdcCfg);
    DDL_ZERO_STRUCT(stcAdcIrq);
    DDL_ZERO_STRUCT(stcAdcIrqCalbaks);
    DDL_ZERO_STRUCT(stcAdcIrqFlag);
    DDL_ZERO_STRUCT(stcAdcAN0Port);
#if(BOARD_VERSION == BOARD_EVT_VERSION)
    Gpio_SetAnalogMode(GpioPortA, GpioPin0);        //PA00 (AIN0)
    stcAdcAN0Port.enDir = GpioDirIn;
    Gpio_Init(GpioPortA, GpioPin0, &stcAdcAN0Port);
#else
    Gpio_SetAnalogMode(GpioPortB, GpioPin12);        //PB12 (AIN19)
    stcAdcAN0Port.enDir = GpioDirIn;
    Gpio_Init(GpioPortB, GpioPin12, &stcAdcAN0Port);
#endif
    if (Ok != Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE))
    {
       ;// return 1;
    }  	

    //ADC配置
    Adc_Enable();
    M0P_BGR->CR_f.BGR_EN = 0x1u;                 //BGR必须使能
    M0P_BGR->CR_f.TS_EN = 0x0u;
    delay100us(1);
    
    stcAdcCfg.enAdcOpMode = AdcSglMode;//AdcSCanMode;         //扫描模式
    stcAdcCfg.enAdcClkDiv = AdcClkSysTDiv1;
    stcAdcCfg.enAdcSampTimeSel = AdcSampTime8Clk;
    stcAdcCfg.enAdcRefVolSel   = RefVolSelAVDD;
    stcAdcCfg.bAdcInBufEn      = FALSE;
    
    Adc_Init(&stcAdcCfg);

	Adc_ConfigSglMode(&stcAdcCfg);
#if(BOARD_VERSION == BOARD_EVT_VERSION)
	Adc_ConfigSglChannel(AdcExInputCH0);
#else
	Adc_ConfigSglChannel(AdcExInputCH19);
#endif
    EnableNvic(ADC_IRQn, IrqLevel1, TRUE);
        
    Adc_EnableIrq();
    
    stcAdcIrq.bAdcIrq = TRUE;
	
    stcAdcIrqCalbaks.pfnAdcIrq = AdcContIrqCallback;
    
    Adc_ConfigIrq(&stcAdcIrq, &stcAdcIrqCalbaks);
      
    //Adc_SGL_Start();

}
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)

void Init_PWM(void)
{
    uint16_t                        u16ArrValue;
    uint16_t                        u16CompareAValue;
    uint16_t                        u16CompareBValue;
    uint16_t                        u16CntValue;
    uint8_t                         u8ValidPeriod;

    stc_tim3_mode23_config_t        stcTim3BaseCfg;
    stc_tim3_m23_compare_config_t   stcTim3PortCmpCfg;
    stc_gpio_config_t               stcTIM3Port;
	
    DDL_ZERO_STRUCT(stcTim3BaseCfg);
    DDL_ZERO_STRUCT(stcTIM3Port);
    DDL_ZERO_STRUCT(stcTim3PortCmpCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralTim3, TRUE);   //Timer3外设时钟使能

	stcTIM3Port.enDir  = GpioDirOut;
	Gpio_ClrIO(GpioPortC, GpioPin13);
    Gpio_Init(GpioPortC, GpioPin13, &stcTIM3Port);
    Gpio_SetAfMode(GpioPortC,GpioPin13,GpioAf3);               //PC13设置为TIM3_CH1B	

	stcTim3BaseCfg.enWorkMode    = Tim3WorkMode3;             //三角波模式
    stcTim3BaseCfg.enCT          = Tim3Timer;                 //定时器功能，计数时钟为内部PCLK
    stcTim3BaseCfg.enPRS         = Tim3PCLKDiv2;//Tim3PCLKDiv1;              //PCLK
    //stcTim3BaseCfg.enCntDir      = Tim3CntUp;                 //向上计数，在三角波模式时只读
    stcTim3BaseCfg.enPWMTypeSel  = Tim3IndependentPWM;        //独立输出PWM    //Tim3ComplementaryPWM;
    stcTim3BaseCfg.enPWM2sSel    = Tim3DoublePointCmp;        //单点比较功能
    stcTim3BaseCfg.bOneShot      = FALSE;                     //循环计数
    stcTim3BaseCfg.bURSSel       = FALSE;                     //上下溢更新
    
    stcTim3BaseCfg.pfnTim3Cb  = Tim3Int;                      //中断函数入口
    
    Tim3_Mode23_Init(&stcTim3BaseCfg);                        //TIM3 的模式0功能初始化
	#if(SUPPORT_1KHZ == _ENABLE)
	uiPWMHCnt = 5;
	uiPWMLCnt = 995;
	#else
	uiPWMHCnt = 50;
	uiPWMLCnt = 9950;
	#endif
    u16ArrValue = uiPWMPeriod;
    Tim3_M23_ARRSet(u16ArrValue, TRUE);                       //设置重载值,并使能缓存
    u16CompareAValue = uiPWMHCnt;
    u16CompareBValue = uiPWMLCnt;

    Tim3_M23_CCR_Set(Tim3CCR1A, u16CompareAValue);
    Tim3_M23_CCR_Set(Tim3CCR1B, u16CompareBValue);

	stcTim3PortCmpCfg.enCHxACmpCtrl   = Tim3PWMMode2;         //OCREFA输出控制OCMA:PWM模式2
    stcTim3PortCmpCfg.enCHxAPolarity  = Tim3PortPositive;     //正常输出
    stcTim3PortCmpCfg.bCHxACmpBufEn   = TRUE;                 //A通道缓存控制
    stcTim3PortCmpCfg.enCHxACmpIntSel = Tim3CmpIntNone;       //A通道比较控制:无
    
    stcTim3PortCmpCfg.enCHxBCmpCtrl   = Tim3PWMMode2;         //OCREFB输出控制OCMB:PWM模式2(PWM互补模式下也要设置，避免强制输出)
    stcTim3PortCmpCfg.enCHxBPolarity  = Tim3PortPositive;     //正常输出
    stcTim3PortCmpCfg.bCHxBCmpBufEn   = TRUE;                 //B通道缓存控制使能
    stcTim3PortCmpCfg.enCHxBCmpIntSel = Tim3CmpIntNone;       //B通道比较控制:无
    
    Tim3_M23_PortOutput_Config(Tim3CH1, &stcTim3PortCmpCfg);  //比较输出端口配置

    u8ValidPeriod = 1;                                        //事件更新周期设置，0表示三角波每半个周期更新一次，每+1代表延迟半个周期
    Tim3_M23_SetValidPeriod(u8ValidPeriod);                   //间隔周期设置
    
    u16CntValue = 0;
    
    Tim3_M23_Cnt16Set(u16CntValue);                           //设置计数初值
    
    Tim3_ClearAllIntFlag();                                   //清中断标志
    Tim3_Mode23_EnableIrq(Tim3UevIrq);                        //使能TIM3 UEV更新中断
    //EnableNvic(TIM3_IRQn, IrqLevel0, TRUE);                   //TIM3中断使能
    
    //Tim3_M23_EnPWM_Output(TRUE, FALSE);                       //端口输出使能
    
    //Tim3_M23_Run();                                           //运行。
    
}
#endif
void MCU_Initialize(void)
{
	Init_OscFrequency();
	Init_IOPorts();
	Init_Timer();
	Init_ADC();
	delay10us(10);
#if(POWER_BOARD_VERSION == POWER_MOS_VERSION)	
	Init_PWM();
#endif	
	Init_APP_Uart();
	EUSART_WIFI_INIT();
}	
void MCUStopOutput(void)
{
	PWMStop();
	ADCStop();
	StopTimer1();
	Gpio_ClrIO(GpioPortC, GpioPin13);
	Gpio_ClrIO(GpioPortD, GpioPin0);
	Gpio_ClrIO(GpioPortD, GpioPin1);

	//LED D1--> D12
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
