/******************************************************************************
 * mcu.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     mcu include file
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#ifndef _MCU_H_

#define _MCU_H_
extern unsigned short int u16AdcRestult0;

extern void MCU_Initialize(void);
extern void ADCStart(void);
extern void ADCStop(void);
extern void StartTimer1(void);
extern void StopTimer1(void);
extern void PWMStart(void);
extern void PWMStop(void);
extern void ALLStart(void);
extern void ALLStop(void);
extern void MCUStopOutput(void);

//extern void Tim0Int(void);
#endif /* _MCU_H_ */

