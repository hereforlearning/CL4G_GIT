/******************************************************************************
 * global.h 
 * 
 * Copyright(C), 2020-2030, SHENZHEN AIAQUA INTERNATIONAL PTY LIMITED
 * 
 * Description: 
 *     global const and varible define
 * Modification History
 * --------------------
 * V1.00, 18 Mar 2020, written by Wingo Wan
 * --------------------
 ******************************************************************************/

#ifndef _GLOBAL_H_

#define _GLOBAL_H_

#define BIT0	                                 0x0001
#define BIT1	                                 0x0002
#define BIT2	                                 0x0004
#define BIT3	                                 0x0008
#define BIT4	                                 0x0010
#define BIT5	                                 0x0020
#define BIT6	                                 0x0040
#define BIT7	                                 0x0080
#define BIT8	                                 0x0100
#define BIT9	                                 0x0200
#define BIT10	                                 0x0400
#define BIT11	                                 0x0800
#define BIT12	                                 0x1000
#define BIT13	                                 0x2000
#define BIT14	                                 0x4000
#define BIT15	                                 0x8000

#define _ON 		                             1
#define _OFF		                             0

#define _ENABLE                                  1
#define _DISABLE                                 0
 
#define _HIGH                                    1
#define _LOW                                     0

#define _INPUT                                   1
#define _OUTPUT                                  0

#define _TRUE                                    1
#define _FALSE                                   0

#define _LEFT_ALIGN		                         0
#define _RIGHT_ALIGN		                     1
#define _CENTER_ALIGN		                     2

#define _FORWARD                                 1
#define _REVERSE                                 0

#define USER_MAX                                 100
#define USER_MIN                                 0

#define CELL_SENSOR                              0
#define EXT_SENSOR                               1

#define TIME_1HOURS                              12
#define TIME_2HOURS                              24
#define TIME_5HOURS                              60
#define TIME_8HOURS                              90
#define TIME_7P5HOURS                            96
#define TIME_10HOURS                             120

#define _EU_DAVEY_                               0
#define _AU_CAS_                                 1

#define BRAND_SELECT                             _EU_DAVEY_

#if(BRAND_SELECT == _EU_DAVEY_)
#define MODEL_8G_2A                              0
#define MODEL_24G_4A                             1

#define MODEL_NUMBER_MIN                         MODEL_8G_2A
#define MODEL_NUMBER_MAX                         MODEL_24G_4A

#define DEFUALT_MODEL                            MODEL_24G_4A
#define SW                                       "V1.6"

#define DEFUALT_CELL_POLARITY                    _FORWARD

#define DEFAULT_CELL_CURRENT                     100

#define DEFAULT_CELL_REVERSE                     TIME_8HOURS  


#define WATER_SENSOR_TYPE                        EXT_SENSOR

#define KEY_REPEAT_FUNCTION                      _DISABLE
#define SUPPORT_REVERSE_SETTING                  _ENABLE

#define WATER_FLOW_CHECK                         _ENABLE
#define POWER_BOARD_TEST                         _DISABLE
#define SYSTEM_STATUS_CHECK                      _DISABLE
#define RUNNING_TIME_DISPLAY                     _ENABLE
#define CURRENT_CALIBRATION                      _ENABLE
#define SW_VERSION_DISPLAY                       _ENABLE
#define AC_POWER_CHECK                           _DISABLE
#define OUTPUT_VOLTAGE_CHECK                     _DISABLE
#define LOW_SALT_CHECK                           _ENABLE
#define SUPPORT_FAST_REVERSE                     _ENABLE
#else

#define MODEL_CSM15                              0
#define MODEL_CSM25                              1
#define MODEL_CSM35                              2
#define MODEL_CSM45                              3
#define MODEL_CSM55                              4
#define MODEL_CSM65                              5

#define MODEL_NUMBER_MIN                         MODEL_CSM15
#define MODEL_NUMBER_MAX                         MODEL_CSM65

#define DEFUALT_MODEL                            MODEL_CSM45//MODEL_CSM25
#define SW                                       "V1.7"

#define DEFUALT_CELL_POLARITY                    _FORWARD

#define DEFAULT_CELL_CURRENT                     100

#define DEFAULT_CELL_REVERSE                     TIME_7P5HOURS

#define WATER_SENSOR_TYPE                        CELL_SENSOR

#define KEY_REPEAT_FUNCTION                      _DISABLE

//#define USE_DYNAMIC_ZERO_REFERENCE 		     _DISABLE
#define SUPPORT_REVERSE_SETTING                  _ENABLE
#define WATER_FLOW_CHECK                         _ENABLE //
#define POWER_BOARD_TEST                         _DISABLE//_ENABLE//
#define SYSTEM_STATUS_CHECK                      _DISABLE
#define RUNNING_TIME_DISPLAY                     _ENABLE
#define CURRENT_CALIBRATION                      _ENABLE
#define SW_VERSION_DISPLAY                       _ENABLE
#define AC_POWER_CHECK                           _DISABLE//_ENABLE
#define OUTPUT_VOLTAGE_CHECK                     _DISABLE
#define LOW_SALT_CHECK                           _DISABLE
#endif
#define BOARD_LED_BUTTON                         0
#define BOARD_LCD_ENCODER                        1

//#define BOARD_SELECT                           BOARD_LED_BUTTON
#define BOARD_EVT_VERSION                        0
#define BOARD_DVT_VERSION                        1
#define BOARD_VERSION                            BOARD_DVT_VERSION//BOARD_EVT_VERSION

#define POWER_MOS_VERSION						 0
#define POWER_TRC_VERSION                        1
#define POWER_BOARD_VERSION                      POWER_TRC_VERSION


#define SUPPORT_1KHZ                             0
#if(SUPPORT_1KHZ == _ENABLE)
#define PWM_PERIOD                               1000
#define PWM_MIN                                  0
#define PWM_MAX                                  1000
#define PWM_CNT_MIN                              0
#define PWM_CNT_MAX                              1000
#else
#define PWM_PERIOD                               10000
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
#define PWM_MIN                                  1300
#define PWM_MAX                                  8500
#define PWM_CNT_MIN                              1300
#define PWM_CNT_MAX                              10000
#else
#define PWM_MIN                                  0
#define PWM_MAX                                  10000
#define PWM_CNT_MIN                              0
#define PWM_CNT_MAX                              10000
#endif
#endif
#if(POWER_BOARD_VERSION == POWER_TRC_VERSION)
#define ADCMAXCNT                                80
#else
#define ADCMAXCNT                                9
#endif

#define DEBUG                                    _DISABLE//_ENABLE 
#define EMC_TEST                                 (DEBUG&_ENABLE)
#define CURRENT_TEST                             (DEBUG&_DISABLE) 
#define BURN_IN_TEST                             (DEBUG&_DISABLE)
#define DIRECTION_TEST                           (DEBUG&_DISABLE)

#endif /* _GLOBAL_H_ */


