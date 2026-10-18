#include "rtc.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "beep.h"
#include "gui.h"
#include "24cxx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//RTC ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/5
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//********************************************************************************
//�޸�˵��
//V1.1 20140726
//����:RTC_Get_Week����,���ڸ�����������Ϣ,�õ�������Ϣ.
////////////////////////////////////////////////////////////////////////////////// 
extern int alarm_allow;
extern int alarm_clock_working;
extern struct alarm_clock my_alarm_clock;
extern void show_date(void);
extern void show_time(void);
extern void draw_time(void);
extern void show_temprature(void);
extern int now;
NVIC_InitTypeDef   NVIC_InitStructure;

//RTCʱ������
//hour,min,sec:Сʱ,����,����
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	ErrorStatus sta;
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	sta=RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	set_alarm_clock(get_next_alarm_clock());
	return sta;
	
}
//RTC��������
//year,month,date:��(0~99),��(1~12),��(0~31)
//week:����(1~7,0,�Ƿ�!)
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	ErrorStatus sta;
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	sta=RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
	set_alarm_clock(get_next_alarm_clock());
	return sta;
}

//RTC��ʼ��
//����ֵ:0,��ʼ���ɹ�;
//       1,LSE����ʧ��;
//       2,�����ʼ��ģʽʧ��;
void init_alarm_clock(void);
u8 My_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry;
	u8 sync_div=0xFF;		//同步分频系数
	u8 async_div=0x7F;		//异步分频系数
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问
	init_alarm_clock();
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//是否第一次配置?(VBAT保持住则跳过,保留原时间)
	{
		RCC_LSEConfig(RCC_LSE_ON);//LSE 开启
		//LSE 起振较慢(常需1~2秒),给足够长的等待时间,优先用 LSE,
		//因为只有 LSE 在 VBAT 域,断主电后才能靠纽扣电池继续走时。
		//retry 真递减做超时,外部晶振彻底坏掉时才退出(不会死等)。
		retry=0;
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET && retry<5000)
		{
			retry++;
			delay_ms(1);				//最长约 5 秒等 LSE 起振
		}
		if(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET)	//LSE 实在起不来,回退 LSI(保证不卡死)
		{
			RCC_LSICmd(ENABLE);
			retry=0;
			while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET && retry<5000)
			{
				retry++;
				delay_ms(1);
			}
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		//RTC时钟源=LSI(注意:LSI断主电即停,无法断电保持)
			async_div=0x7C;	//LSI约32KHz: (124+1)*(255+1)=32000
			sync_div =0xFF;
		}
		else
		{
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//RTC时钟源=LSE(VBAT域,断电可走时)
			async_div=0x7F;	//LSE 32.768KHz: (127+1)*(255+1)=32768
			sync_div =0xFF;
		}
		RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟

    RTC_InitStructure.RTC_AsynchPrediv = async_div;//RTC异步分频系数
    RTC_InitStructure.RTC_SynchPrediv  = sync_div; //RTC同步分频系数
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//24小时格式
    RTC_Init(&RTC_InitStructure);

		RTC_Set_Time(10,43,1,RTC_H12_AM);	//仅首次配置时设默认时间
		RTC_Set_Date(25,6,23,1);		//仅首次配置时设默认日期

		//写入标志,标记已初始化。下次上电只要 VBAT 保持住 RTC 域,
		//就会读到 0x5050 而跳过上面整段,从而沿用电池保持的当前时间。
		//即使回退了 LSI 也写标志:否则每次上电都会把时间重置成 10:43。
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);
	}

	//RTC闹钟,根据EEPROM恢复闹钟设置,重设下一次闹醒的硬件中断
	if(my_alarm_clock.point>0)
		set_alarm_clock(get_next_alarm_clock());

	return 0;
}

//��������ʱ��(����������,24Сʱ��)
//week:���ڼ�(1~7) @ref  RTC_Alarm_Definitions
//hour,min,sec:Сʱ,����,����
void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);//�ر�����A 
	
  RTC_TimeTypeInitStructure.RTC_Hours=hour;//Сʱ
	RTC_TimeTypeInitStructure.RTC_Minutes=min;//����
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;//��
	RTC_TimeTypeInitStructure.RTC_H12=RTC_H12_AM;
  
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay=week;//����
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel=RTC_AlarmDateWeekDaySel_WeekDay;//��������
	RTC_AlarmTypeInitStructure.RTC_AlarmMask=RTC_AlarmMask_None;//��ȷƥ�����ڣ�ʱ����
	RTC_AlarmTypeInitStructure.RTC_AlarmTime=RTC_TimeTypeInitStructure;
  RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&RTC_AlarmTypeInitStructure);
 
	
	RTC_ClearITPendingBit(RTC_IT_ALRA);//���RTC����A�ı�־
  EXTI_ClearITPendingBit(EXTI_Line17);//���LINE17�ϵ��жϱ�־λ 
	
	RTC_ITConfig(RTC_IT_ALRA,ENABLE);//��������A�ж�
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);//��������A 
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;//LINE17
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE17
  EXTI_Init(&EXTI_InitStructure);//����

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}

//�����Ի��Ѷ�ʱ������  
/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:�Զ���װ��ֵ.����0,�����ж�.
void RTC_Set_WakeUp(u32 wksel,u16 cnt)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);//�ر�WAKE UP
	
	RTC_WakeUpClockConfig(wksel);//����ʱ��ѡ��
	
	RTC_SetWakeUpCounter(cnt);//����WAKE UP�Զ���װ�ؼĴ���
	
	
	RTC_ClearITPendingBit(RTC_IT_WUT); //���RTC WAKE UP�ı�־
  EXTI_ClearITPendingBit(EXTI_Line22);//���LINE22�ϵ��жϱ�־λ 
	 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);//����WAKE UP ��ʱ���ж�
	RTC_WakeUpCmd( ENABLE);//����WAKE UP ��ʱ����
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;//LINE22
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE22
  EXTI_Init(&EXTI_InitStructure);//����
 
 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}

//RTC�����жϷ�����
void RTC_Alarm_IRQHandler(void)
{ int length;
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A�ж�?
	{
		alarm_clock_working=1;
		length=50;
		RTC_ClearFlag(RTC_FLAG_ALRAF);//����жϱ�־
		alarm_allow=1;
		BEEP=1;
		while(alarm_allow&&length--){
			if(now==CLOCK){
			show_date();
			show_time();
			draw_time();
			show_temprature();
			}
			BEEP=!BEEP;
			LED1=!LED1;
			delay_ms(100);
		}
		BEEP=0;
		LED1=1;
		alarm_clock_working=0;
		set_alarm_clock(get_next_alarm_clock());
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//����ж���17���жϱ�־ 											 
}

//RTC WAKE UP�жϷ�����
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF)==SET)//WK_UP�ж�?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	//����жϱ�־
	}    
	EXTI_ClearITPendingBit(EXTI_Line22);//����ж���22���жϱ�־ 								
}
 
//����ʱ��
struct time time_begin={0,0,0};
struct alarm_clock my_alarm_clock;
int seconds_between(struct time begin,struct time end){//��ȡ��ʱ��֮�������
	  struct time temp1=begin,temp2=end;
	  int length=0; 
	  if(temp1.week==0&&temp2.week==0){
			 length=(temp2.hour-temp1.hour)*60*60+(temp2.minute-temp1.minute)*60;
			 if(length<0)
				 length+=60*60*24;
				 
		}
	  else if(temp1.week==0||temp2.week==0){
		    length=(temp2.hour-temp1.hour)*60*60+(temp2.minute-temp1.minute)*60;
			if(length<0)
				 length+=60*60*24;
		}
		else{
		   length=(temp2.week-temp1.week)*60*60*24+(temp2.hour-temp1.hour)*60*60+(temp2.minute-temp1.minute)*60;
				if(length<0)
				 length+=60*60*24*7;
		}
		return length;
}
void copy_to_time(RTC_DateTypeDef const*src,RTC_TimeTypeDef const*src1,struct time*dst){
    dst->week=src->RTC_WeekDay;
	  dst->hour=src1->RTC_Hours;
	  dst->minute=src1->RTC_Minutes;
}
void init_alarm_clock(void){
     my_alarm_clock.point=0;
     my_alarm_clock.maxsize=6;
     load_alarm_clock();//�ϵ��EEPROM�ָ�����
}

//������ǰ�����б�д��EEPROM(AT24C02)
//��ʽ: [0]=ħ��0xA5 [1]=���� [2..]=ÿ������3�ֽ�{week,hour,minute}
void save_alarm_clock(void){
    u8 buf[2+6*3];
    int i;
    buf[0]=0xA5;
    buf[1]=my_alarm_clock.point;
    for(i=0;i<my_alarm_clock.point;i++){
        buf[2+i*3]  =(u8)my_alarm_clock.clock[i].week;
        buf[2+i*3+1]=(u8)my_alarm_clock.clock[i].hour;
        buf[2+i*3+2]=(u8)my_alarm_clock.clock[i].minute;
    }
    AT24CXX_Write(0,buf,2+my_alarm_clock.point*3);
}

//��EEPROM(AT24C02)��������б����ڴ�
void load_alarm_clock(void){
    u8 i,point;
    if(AT24CXX_ReadOneByte(0)!=0xA5){//δ��Ч��ʼ��,�����ֿ�
        my_alarm_clock.point=0;
        return;
    }
    point=AT24CXX_ReadOneByte(1);
    if(point>my_alarm_clock.maxsize)point=0;//�����ݱ���
    my_alarm_clock.point=point;
    for(i=0;i<point;i++){
        my_alarm_clock.clock[i].week  =AT24CXX_ReadOneByte(2+i*3);
        my_alarm_clock.clock[i].hour  =AT24CXX_ReadOneByte(2+i*3+1);
        my_alarm_clock.clock[i].minute=AT24CXX_ReadOneByte(2+i*3+2);
    }
}

struct time get_next_alarm_clock(void){//��ȡ��һ��Ҫ���õ�����
    RTC_DateTypeDef RTC_DateStruct;	
	  RTC_TimeTypeDef RTC_TimeStruct;
   	int length,i,min_length;
	  struct time now,dst;
	
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	  RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	
	  
	  copy_to_time(&RTC_DateStruct,&RTC_TimeStruct,&now);
	  min_length=60*60*24*7;
	
	  for(i=0;i<my_alarm_clock.point;i++){
			
	  length=seconds_between(now,my_alarm_clock.clock[i]);
		
		if(length!=0&&length<min_length){
			
		min_length=length;
		
		dst=my_alarm_clock.clock[i];
		}
		}
	  
		return dst;
}
void set_alarm_clock(struct time temp){
    RTC_DateTypeDef RTC_DateStruct;	
	  RTC_TimeTypeDef RTC_TimeStruct;
	  int week=temp.week;
   	struct time now;
	 
	  
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	  RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	
	 
	  copy_to_time(&RTC_DateStruct,&RTC_TimeStruct,&now); 
	  if(week==0){
		  if(now.hour>temp.hour||(now.hour==temp.hour&&now.minute>temp.minute)){
			week=now.week+1;
			}
			else{
			week=now.week;
			}
		} 
	  RTC_Set_AlarmA(week,temp.hour,temp.minute,0);
	  
}
int allow_adding(void){//�Ƿ���������
	
return my_alarm_clock.point<my_alarm_clock.maxsize;
}
int not_exist(struct time temp){//�������������
	int i;
  for(i=0;i<my_alarm_clock.point;i++){
	 if(temp.week==my_alarm_clock.clock[i].week&&temp.hour==my_alarm_clock.clock[i].hour&&temp.minute==my_alarm_clock.clock[i].minute){
	   return 0;
	 }
  }
	return 1;
}
void add_alarm_clock(struct time temp){//����һ������
	//set_alarm_clock(temp);
	//my_alarm_clock.maxsize=10;
if(allow_adding()&&not_exist(temp)){
my_alarm_clock.clock[my_alarm_clock.point++]=temp;
if(my_alarm_clock.point==1)
	set_alarm_clock(temp);
else{
  set_alarm_clock(get_next_alarm_clock());
}
save_alarm_clock();//�־û���EEPROM
}/**/
}
void drop_alarm_clock(int i){
for(;i<my_alarm_clock.point-1;i++){
my_alarm_clock.clock[i]=my_alarm_clock.clock[i+1];
}
my_alarm_clock.point--;

if(my_alarm_clock.point==0)
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
else
  set_alarm_clock(get_next_alarm_clock());
save_alarm_clock();//�־û���EEPROM
}












