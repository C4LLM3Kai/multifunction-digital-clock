#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"
#include "lcd.h"
#include "usmart.h"
#include "rtc.h"
#include "math.h"
#include "gui.h"
#include "adc.h"
#include "beep.h"
#include "exti.h"
#include "tpad.h"
#include "touch.h"
#include "key.h"
#include "24cxx.h"
//ALIENTEK ̽����STM32F407������ ʵ��15
//RTCʵʱʱ�� ʵ�� -�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//�������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK
int main(void)
{ 
	//u8 t=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);      //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	BEEP_Init();         	//��ʼ���������˿�
	usmart_dev.init(84); 	//��ʼ��USMART
	LED_Init();					  //��ʼ��LED
	KEY_Init();					  //��ʼ��������
	AT24CXX_Init();				//��ʼ��EEPROM(IIC),������ڹ�����ǰ
 	LCD_Init();					  //��ʼ��LCD
	My_RTC_Init();		 		//��ʼ��RTC(�ڲ�����EEPROM���ع���)
  Adc_Init();         //�ڲ��¶ȴ�����ADC��ʼ��
	EXTIX_Init();       //��ʼ���ⲿ�ж����� 
	tp_dev.init();
	run_menu();

  while(1);
}
