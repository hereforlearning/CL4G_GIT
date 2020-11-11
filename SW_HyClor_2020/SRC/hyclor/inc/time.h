#ifndef TIME_H
#define TIME_H
#if USE_NEW_TIME_JUDGE
extern	unsigned char time_index_now;
#endif

enum
{
    weekday1=0,
    weekday2,
    weekday3,
    weekday4,
    weekday5,
    weekday6,
    weekday7
};
extern unsigned char Weekday[];
/***********************************************************************/
//定时器部�?
typedef struct
{
	unsigned char b0 : 1;
	unsigned char b1 : 1;
	unsigned char b2 : 1;
	unsigned char b3 : 1;
	unsigned char b4 : 1;
	unsigned char b5 : 1;
	unsigned char b6 : 1;
	unsigned char b7 : 1;
} _bitss;
extern _bitss flag; 

/***********************************************************************/
#define     TIMER_SET_NULL			0x00
#define     TIMER_1_ON				0x01//T1
#define     TIMER_1_OFF				0x02
#define     TIMER_2_ON				0x03//T2
#define     TIMER_2_OFF				0x04
#define     TIMER_3_ON				0x05//T3
#define     TIMER_3_OFF				0x06
#define     TIMER_4_ON				0x07//T4
#define     TIMER_4_OFF				0x08
#define     TIMER_5_ON				0x09//T5
#define     TIMER_5_OFF				0x0A
#define     TIMER_6_ON				0x0B//T6
#define     TIMER_6_OFF				0x0C
#define     TIMER_7_ON				0x0d//T6
#define     TIMER_7_OFF				0x0e
#define     TIMER_8_ON				0x0f//T6
#define     TIMER_8_OFF				0x10

#define     TIMER_SET_END			0x11
#define     TIMER_SET_DEBUGE		0x12

typedef struct 
{	
	unsigned char TIMER_DISPLA_ON;//��Ӧ��������ʾ
	unsigned char TIMER_DISPLA_OFF;
	unsigned char Day_ON;
	unsigned char Day_OFF;
	unsigned char Hour_ON;
	unsigned char Hour_OFF;
	unsigned char Min_ON;	
	unsigned char Min_OFF;			
}TIMER;

#define T7 Timer_Buffer[6] 
#define T8 Timer_Buffer[7]

/*extern TIMER T1,T2,T3,T4,T5,T6;*/

#if USE_CLOCK_DIV4
#define KEY_DALAY_TIME 5
#else 
#define KEY_DALAY_TIME 20
#endif /*USE_CLOCK_DIV4*/

#if USE_CLOCK_DIV4
#define KEY_DALAY_REPEAT_TIME 50
#else 
#define KEY_DALAY_REPEAT_TIME 200
#endif /*USE_CLOCK_DIV4*/
#if USE_CLOCK_DIV4
#define KEY_DALAY_REPEAT_START_TIME 40
#else 
#define KEY_DALAY_REPEAT_START_TIME 40
#endif /*USE_CLOCK_DIV4*/
#if USE_CLOCK_DIV4
#define KEY_DALAY_TIME_START_TIME 34
#else 
#define KEY_DALAY_TIME_START_TIME 135
#endif /*USE_CLOCK_DIV4*/

extern TIMER Timer_Buffer[8];
extern unsigned char RUN_MIN;
extern unsigned int  RUN_HOUR;

extern unsigned char ucDay;//星期值变�?
extern unsigned char ucHour;//小时
extern unsigned char ucMin;//分钟
extern unsigned char ucSec;//秒钟
extern unsigned char uc_TIMERn;
extern unsigned char TIMER_Groud;
extern unsigned char TIMER_DAY;
extern unsigned char CLOCK_DAY;
extern unsigned char TIMER_DISPLA_N;
extern unsigned char TIMER_DISPLA_HOUR;
extern unsigned char TIMER_DISPLA_MIN;
extern unsigned char CLOCK_MOD_DISPLA;
extern unsigned char CLOCK_MOD_N;
extern unsigned int  RUN_SEC;
extern unsigned char EXIT_TIMER_SEC;
extern unsigned char RUN_MIN;
extern unsigned int  RUN_HOUR;


extern unsigned char ucDay;
extern unsigned char ucHour;
extern unsigned char ucMin;
extern unsigned char ucSec;
extern _bitss flag; 
#define 	Flag_1s 				flag.b0
#define		ucTimer_Flag 			flag.b1
#define		FLAG_5MIN 				flag.b2
#define     FLAG_1MIN				flag.b3
#define     FLAG_SINGLE				flag.b4
#define     FLAG_GROUP_2			flag.b5

/*#define     FLAG_ALL_WEEK			flag.b6*/
extern unsigned char uc_TIMERn;
extern unsigned char TIMER_Groud;//定时器显示的组数
extern unsigned char TIMER_DAY;//定时器星期码值
extern unsigned char CLOCK_DAY;//时钟的星期码值，这两个用于&后判断哪天开启，哪天关闭 
extern unsigned char TIMER_DISPLA_N;//定时器星期设置的显示组数
extern unsigned char TIMER_DISPLA_HOUR;
extern unsigned char TIMER_DISPLA_MIN;
extern unsigned char CLOCK_MOD_DISPLA;
extern unsigned char CLOCK_MOD_N;
extern TIMER Timer_Buffer[8];
extern unsigned char EXIT_TIMER_SEC;
extern unsigned char RUN_MIN;
//extern void Delay_ms(unsigned long count);//移植改变
void Relay_set(unsigned out);
void RtcCycCb(void);// interrupt of rtc
void time_init(void);
void rtc_int(void);
void MOD_CHOOSE(void);
void RUN_CLOCK_EEPROM(void);
void time_process(void); 
#endif
