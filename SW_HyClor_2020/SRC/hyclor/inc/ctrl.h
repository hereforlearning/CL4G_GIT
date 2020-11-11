/******************************************************************************
 * ctrl.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     ctrl include file
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/
#ifndef _CTRL_H_
		  
#define _CTRL_H_

extern unsigned int uiPWMHCnt;
extern unsigned int uiPWMLCnt;
extern unsigned int uiPWMPeriod;
extern unsigned char bTimer3EnableFlag;
extern unsigned char bStopOutputFlag;

extern void Delay_ms(unsigned int iCnt);
extern void PowerOnInitialize(void);
extern void ProcessHandler(void);
extern void CellPolarityReverse(void);
extern void ModelSelect(void);
extern void CellOutputCurrentAdjust(unsigned char ucAdjDirect);
extern void ComboKeyProcess(void);
extern void SpecialSecretCodeProcess(unsigned char ucKey);
extern void MainTimerHandler(void);
extern void CurrentADCProcess(void);
#endif
