#include "gui.h"
u16 bk_color = THEME_BG;
const float PI=3.1415926;
int now;
int confirm=1;
int change_hour=1;
int add=0;
int sub=0;
int alarm_allow=1;
char usart_data[20];
int pos=0;
int input=0;
int alarm_clock_working=0;
int dropi=0;
int last_dropi=0;
int continue_=0;
int stop=0;
int restart=0;
int hour_count_down;
int minute_count_down;
int second_count_down;
int preset_pick=-1;
int lap_times[5][3];
int lap_count=0;
int lap_flag=0;
int last_chime_hour=-1;   // 上次整点报时的小时,-1 表示尚未报/已复位

extern struct alarm_clock my_alarm_clock;
struct button set_time_button;
struct button set_clock_button;
struct button countdown_button;
struct button count_button;
struct button set_time_port_button;
struct button set_time_click_button;
struct button add_button1;
struct button add_button2;
struct button add_button3;
struct button add_button4;
struct button add_button5;
struct button add_button6;
struct button sub_button1;
struct button sub_button2;
struct button sub_button3;
struct button sub_button4;
struct button sub_button5;
struct button sub_button6;
struct button confirm_button;
struct button cancel_button;
struct button drop_button;
struct button add_button;
struct button alarm_clock_list[6];
struct button continue_button;
struct button stop_button;
struct button restart_button;
struct button lap_button;
struct button preset_button[5];
const int preset_minutes[5]={1,3,5,10,30};

void draw_point(int x1,int y1,u16 color){
	POINT_COLOR=color;
	LCD_DrawPoint(x1,y1);
}
void fill_circle(int uRow,int uCol,int size,u16 color){
  int i=0;   
	POINT_COLOR=color;
	for(;i<=size;i++){
	LCD_Draw_Circle(uRow,uCol,size-i);
	}
	LCD_DrawRectangle(uRow,uCol,uRow+1,uCol+1);
}
void draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color){
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return;
	if(size==0){draw_point(x1,y1,color);return;} 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance){xerr-=distance;uRow+=incx;}  
		if(yerr>distance){yerr-=distance;uCol+=incy;}
	}  
} 

void back(void){
		LCD_Clear(bk_color);
	  BACK_COLOR=bk_color;
	      
}
void clock(void){
	  int i,midx=250,midy=250,r=200;
	  int x,y,x1,y1;
	  float ang;
	  u16 numbuf;
	  back();
	  // ---- layered bezel: dark outer ring -> soft mid ring -> white face ----
	  POINT_COLOR=THEME_DIAL_BEZEL;
	  for(i=0;i<8;i++) LCD_Draw_Circle(midx,midy,r+i);        // thick dark bezel
	  POINT_COLOR=THEME_DIAL_RING2;
	  for(i=0;i<3;i++) LCD_Draw_Circle(midx,midy,r-i);        // soft inner ring of bezel
	  // white dial face
	  fill_circle(midx,midy,r-4,THEME_DIAL_FACE);
	  // faint inset ring on the face, just inside the ticks
	  POINT_COLOR=THEME_DIAL_INSET;
	  LCD_Draw_Circle(midx,midy,r-44);

	  // ---- 60 minute ticks (minor) + 12 hour ticks (major) ----
	  for(i=0;i<60;i++){
	    ang=PI*2*i/60-PI/2;
	    if(i%5==0){
	      // major tick: longer & thicker, dark
	      x =midx+(int)((r-12)*cos(ang));
	      y =midy+(int)((r-12)*sin(ang));
	      x1=midx+(int)((r-34)*cos(ang));
	      y1=midy+(int)((r-34)*sin(ang));
	      draw_bline(x,y,x1,y1,2,THEME_TICK_MAJOR);
	    }else{
	      // minor tick: short & thin, light
	      x =midx+(int)((r-12)*cos(ang));
	      y =midy+(int)((r-12)*sin(ang));
	      x1=midx+(int)((r-22)*cos(ang));
	      y1=midy+(int)((r-22)*sin(ang));
	      draw_bline(x,y,x1,y1,0,THEME_TICK_MINOR);
	    }
	  }

	  // ---- hour numbers 1..12, centered at each major tick ----
	  POINT_COLOR=THEME_DIAL_NUM;
	  BACK_COLOR=THEME_DIAL_FACE;
	  for(i=1;i<=12;i++){
	    ang=PI*2*i/12-PI/2;                 // 12 on top, 3 on right
	    x =midx+(int)((r-62)*cos(ang));
	    y =midy+(int)((r-62)*sin(ang));
	    numbuf=i;
	    // center the glyphs: 24px font -> 12px per digit, 24px tall
	    x-=(numbuf>9?12:6);
	    y-=12;
	    LCD_ShowNum(x,y,numbuf,numbuf>9?2:1,24);
	  }
	  BACK_COLOR=bk_color;
}

// info area geometry (kept in sync with the panel in show_clock)
#define INFO_X    125
#define INFO_X1   475
#define INFO_R1Y  478   // date row top
#define INFO_R2Y  540   // time row top
#define INFO_R3Y  602   // temp row top
#define INFO_ROWH 54    // colored tag block height
#define INFO_TAGW 96    // colored tag block width
#define INFO_VALX 234   // value text left edge

// draw one colored tag block with a white centered label
static void draw_info_tag(int ytop,u16 color,const char*label){
	  int len=strlen(label);
	  int tx=INFO_X+8+(INFO_TAGW-len*12)/2;
	  int ty=ytop+(INFO_ROWH-24)/2;
	  if(tx<INFO_X+10)tx=INFO_X+10;
	  LCD_Fill(INFO_X+8,ytop,INFO_X+8+INFO_TAGW,ytop+INFO_ROWH,color);
	  POINT_COLOR=THEME_INFO_LABEL;
	  BACK_COLOR=color;
	  LCD_ShowString(tx,ty,INFO_TAGW,24,24,(u8*)label);
	  BACK_COLOR=bk_color;
}
// paint the static look of the date/time/temperature panel once
void draw_info_frame(void){
	  draw_panel(INFO_X,470,INFO_X1,656);
	  // row divider lines
	  POINT_COLOR=THEME_INFO_DIV;
	  LCD_DrawLine(INFO_X+8,INFO_R2Y-4,INFO_X1-8,INFO_R2Y-4);
	  LCD_DrawLine(INFO_X+8,INFO_R3Y-4,INFO_X1-8,INFO_R3Y-4);
	  // colored tag blocks
	  draw_info_tag(INFO_R1Y,THEME_INFO_DATE,"DATE");
	  draw_info_tag(INFO_R2Y,THEME_INFO_TIME,"TIME");
	  draw_info_tag(INFO_R3Y,THEME_INFO_TEMP,"TEMP");
}
void show_date(void){
	  u8 tbuf[32];
		RTC_DateTypeDef RTC_DateStruct;
	  const char*wk[8]={"","Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
	  int wd;
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	  wd=RTC_DateStruct.RTC_WeekDay; if(wd<1||wd>7)wd=0;
	  POINT_COLOR=THEME_INFO_VALUE;
	  BACK_COLOR=THEME_PANEL;
	  sprintf((char*)tbuf,"20%02d-%02d-%02d %s",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,wk[wd]);
	  LCD_ShowString(INFO_VALX,INFO_R1Y+(INFO_ROWH-24)/2,INFO_X1-INFO_VALX-6,24,24,tbuf);
	  BACK_COLOR=bk_color;
}
void show_time(void){
	    RTC_TimeTypeDef RTC_TimeStruct;
	    u8 tbuf[24];
      RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
			POINT_COLOR=THEME_INFO_VALUE;
			BACK_COLOR=THEME_PANEL;
			sprintf((char*)tbuf,"%02d:%02d:%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
			LCD_ShowString(INFO_VALX,INFO_R2Y+(INFO_ROWH-24)/2,INFO_X1-INFO_VALX-6,24,24,tbuf);
			BACK_COLOR=bk_color;
}
// 整点报时：到达新的整点(xx:00)时长鸣一声。
// 用 last_chime_hour 去重,保证每个整点只报一次;闹钟正响时让行。
void hour_chime(void){
	  RTC_TimeTypeDef RTC_TimeStruct;
	  RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	  if(alarm_clock_working){          // 闹钟优先,正在响则不抢蜂鸣器
	    last_chime_hour=-1;             // 让闹钟结束后若仍在整点窗口可补报
	    return;
	  }
	  if(RTC_TimeStruct.RTC_Minutes==0){
	    if(last_chime_hour!=RTC_TimeStruct.RTC_Hours){
	      last_chime_hour=RTC_TimeStruct.RTC_Hours;   // 标记本整点已报
	      // 长鸣一声 "哔——",约 1 秒,同时点亮 LED0
	      BEEP=1; LED0=0;
	      delay_ms(1000);
	      BEEP=0; LED0=1;
	    }
	  }else{
	    last_chime_hour=-1;             // 离开整点这一分钟后复位,准备下一个整点
	  }
}
void draw_time(void){
	   clear_time_area();
	   draw_hour();
	   draw_minute();
	   draw_second();
}

void draw_hour(void){
     RTC_TimeTypeDef RTC_TimeStruct;
	   int midx=250,midy=250;
	   int length=70,x,y;
	   float ang;
     RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	   if(RTC_TimeStruct.RTC_Hours>12)
	   RTC_TimeStruct.RTC_Hours-=12;
	   ang=PI*2*RTC_TimeStruct.RTC_Hours/12+PI*2/12*RTC_TimeStruct.RTC_Minutes/60-PI/2;
	   x=midx+(int)(length*cos(ang));
	   y=midy+(int)(length*sin(ang));
	   draw_bline(x,y,midx,midy,3,THEME_HAND_EDGE);   // white outline
	   draw_bline(x,y,midx,midy,2,THEME_HOUR);        // hour hand body
}
void draw_minute(void){
	   RTC_TimeTypeDef RTC_TimeStruct;
	   int midx=250,midy=250;
	   int length=100,x,y;
	   float ang;
     RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	   ang=PI*2*RTC_TimeStruct.RTC_Minutes/60-PI/2;
	   x=midx+(int)(length*cos(ang));
	   y=midy+(int)(length*sin(ang));
	   draw_bline(x,y,midx,midy,2,THEME_HAND_EDGE);   // white outline
	   draw_bline(x,y,midx,midy,1,THEME_MIN);         // minute hand body
}
void draw_second(void){
	   RTC_TimeTypeDef RTC_TimeStruct;
	   int midx=250,midy=250;
	   int length=108,tail=24,x,y,tx,ty;
	   float ang;
     RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	   ang=PI*2*RTC_TimeStruct.RTC_Seconds/60-PI/2;
	   x =midx+(int)(length*cos(ang));
	   y =midy+(int)(length*sin(ang));
	   tx=midx-(int)(tail*cos(ang));   // counterweight tail (opposite side)
	   ty=midy-(int)(tail*sin(ang));
	   draw_bline(tx,ty,x,y,1,THEME_SEC);          // thin red second hand + tail
	   fill_circle(tx,ty,3,THEME_HAND_TAIL);       // tail weight
	   // center hub cap, drawn last so it sits above all hands
	   fill_circle(midx,midy,5,THEME_CAP_OUT);
	   fill_circle(midx,midy,2,THEME_CAP_IN);
}
void clear_time_area(void){
	  int midx=250,midy=250;
	  // Erase the hand sweep area back to the white dial face.
	  // Use the hardware block-fill (LCD_Fill) instead of fill_circle():
	  // a r=120 filled circle is 121 nested LCD_Draw_Circle calls and is far
	  // too slow to run every 100ms (it starved touch/key sampling and made
	  // the UI appear frozen). A square fill is near-instant. The half-size
	  // (110) covers the longest hand (108px) while its corners (~155px from
	  // center) stay just inside the tick inset ring (156px), so ticks and
	  // numbers are never overwritten.
	  LCD_Fill(midx-110,midy-110,midx+110,midy+110,THEME_DIAL_FACE);
}
void show_temprature(void){
    int temp=Get_Temprate();	// temperature x100
	  u8 tbuf[24];
	  int neg=0;
	  if(temp<0){neg=1;temp=-temp;}
	  POINT_COLOR=THEME_INFO_VALUE;
	  BACK_COLOR=THEME_PANEL;
	  sprintf((char*)tbuf,"%s%d.%02d C",neg?"-":"",temp/100,temp%100);
	  // pad to a fixed width so stale wider readings are fully overwritten
	  LCD_Fill(INFO_VALX,INFO_R3Y+(INFO_ROWH-24)/2,INFO_X1-6,INFO_R3Y+(INFO_ROWH-24)/2+24,THEME_PANEL);
	  LCD_ShowString(INFO_VALX,INFO_R3Y+(INFO_ROWH-24)/2,INFO_X1-INFO_VALX-6,24,24,tbuf);
	  BACK_COLOR=bk_color;
}
void set_time_click_buttons(void){
   int beginx=10,beginy=10,height=100,width=100,between=200,betweeny=10;
   
	
   add_button1.x=beginx;
   add_button1.y=beginy;
   add_button1.x1=beginx+width;
   add_button1.y1=beginy+height;
   add_button1.color=THEME_BTN;
	 strcpy(add_button1.str,"+");
	 draw_button(add_button1);
	
	 sub_button1.x= add_button1.x1+between;
   sub_button1.y= add_button1.y;
   sub_button1.x1=sub_button1.x+width;
   sub_button1.y1=sub_button1.y+height;
   sub_button1.color=THEME_BTN;
	 strcpy(sub_button1.str,"-");
	 draw_button(sub_button1);
	
	 add_button2.x=beginx;
   add_button2.y=add_button1.y1+betweeny;
   add_button2.x1=add_button2.x+width;
   add_button2.y1=add_button2.y+height;
   add_button2.color=THEME_BTN;
	 strcpy(add_button2.str,"+");
	 draw_button(add_button2);
	 
	 sub_button2.x= add_button1.x1+between;
   sub_button2.y= add_button2.y;
   sub_button2.x1=sub_button2.x+width;
   sub_button2.y1=sub_button2.y+height;
   sub_button2.color=THEME_BTN;
	 strcpy(sub_button2.str,"-");
	 draw_button(sub_button2);
	
	 add_button3.x=beginx;
   add_button3.y=add_button2.y1+betweeny;
   add_button3.x1=add_button3.x+width;
   add_button3.y1=add_button3.y+height;
   add_button3.color=THEME_BTN;
	 strcpy(add_button3.str,"+");
	 draw_button(add_button3);
	 
	 sub_button3.x= add_button3.x1+between;
   sub_button3.y= add_button3.y;
   sub_button3.x1=sub_button3.x+width;
   sub_button3.y1=sub_button3.y+height;
   sub_button3.color=THEME_BTN;
	 strcpy(sub_button3.str,"-");
	 draw_button(sub_button3);
	
   add_button4.x=beginx;
   add_button4.y=add_button3.y1+betweeny;
   add_button4.x1=add_button4.x+width;
   add_button4.y1=add_button4.y+height;
   add_button4.color=THEME_BTN;
	 strcpy(add_button4.str,"+");
	 draw_button(add_button4);
	 
	 sub_button4.x= add_button4.x1+between;
   sub_button4.y= add_button4.y;
   sub_button4.x1=sub_button4.x+width;
   sub_button4.y1=sub_button4.y+height;
   sub_button4.color=THEME_BTN;
	 strcpy(sub_button4.str,"-");
	 draw_button(sub_button4);
	 
	 add_button5.x=beginx;
   add_button5.y=add_button4.y1+betweeny;
   add_button5.x1=add_button5.x+width;
   add_button5.y1=add_button5.y+height;
   add_button5.color=THEME_BTN;
	 strcpy(add_button5.str,"+");
	 draw_button(add_button5);
	 
	 sub_button5.x= add_button5.x1+between;
   sub_button5.y= add_button5.y;
   sub_button5.x1=sub_button5.x+width;
   sub_button5.y1=sub_button5.y+height;
   sub_button5.color=THEME_BTN;
	 strcpy(sub_button5.str,"-");
	 draw_button(sub_button5);
	 
	
	 
	 confirm_button.x=beginx;
   confirm_button.y=add_button5.y1+betweeny*2;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_ACCENT;
	 strcpy(confirm_button.str,"confirm");
	 draw_button(confirm_button);
	
	
	 cancel_button.x= confirm_button.x1+between;
   cancel_button.y= confirm_button.y;
   cancel_button.x1=cancel_button.x+width;
   cancel_button.y1=cancel_button.y+height;
   cancel_button.color=THEME_DANGER;
	 strcpy(cancel_button.str,"cancel");
	 draw_button(cancel_button);
}
void set_time_click_click(void){

if(tp_dev.scan(0)){
if(tp_dev.x[0]>add_button1.x&&tp_dev.x[0]<add_button1.x1&&tp_dev.y[0]>add_button1.y&&tp_dev.y[0]<add_button1.y1)
{
  add=1;
}
else if(tp_dev.x[0]>add_button2.x&&tp_dev.x[0]<add_button2.x1&&tp_dev.y[0]>add_button2.y&&tp_dev.y[0]<add_button2.y1)
{
  add=2;
}
else if(tp_dev.x[0]>add_button3.x&&tp_dev.x[0]<add_button3.x1&&tp_dev.y[0]>add_button3.y&&tp_dev.y[0]<add_button3.y1)
{
  add=3;
}
else if(tp_dev.x[0]>add_button4.x&&tp_dev.x[0]<add_button4.x1&&tp_dev.y[0]>add_button4.y&&tp_dev.y[0]<add_button4.y1)
{
  add=4;
}
else if(tp_dev.x[0]>add_button5.x&&tp_dev.x[0]<add_button5.x1&&tp_dev.y[0]>add_button5.y&&tp_dev.y[0]<add_button5.y1)
{
 add=5;
}
else if(tp_dev.x[0]>sub_button1.x&&tp_dev.x[0]<sub_button1.x1&&tp_dev.y[0]>sub_button1.y&&tp_dev.y[0]<sub_button1.y1)
{
  sub=1;
}
else if(tp_dev.x[0]>sub_button2.x&&tp_dev.x[0]<sub_button2.x1&&tp_dev.y[0]>sub_button2.y&&tp_dev.y[0]<sub_button2.y1)
{
  sub=2;
}
else if(tp_dev.x[0]>sub_button3.x&&tp_dev.x[0]<sub_button3.x1&&tp_dev.y[0]>sub_button3.y&&tp_dev.y[0]<sub_button3.y1)
{
  sub=3;
}
else if(tp_dev.x[0]>sub_button4.x&&tp_dev.x[0]<sub_button4.x1&&tp_dev.y[0]>sub_button4.y&&tp_dev.y[0]<sub_button4.y1)
{
  sub=4;
}
else if(tp_dev.x[0]>sub_button5.x&&tp_dev.x[0]<sub_button5.x1&&tp_dev.y[0]>sub_button5.y&&tp_dev.y[0]<sub_button5.y1)
{
  sub=5;
}
else if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
	confirm=1;
  now=CLOCK;
}
else if(tp_dev.x[0]>cancel_button.x&&tp_dev.x[0]<cancel_button.x1&&tp_dev.y[0]>cancel_button.y&&tp_dev.y[0]<cancel_button.y1)
{
  confirm=0;
  now=CLOCK;
}
}
}
int get_max_day(int year,int month){
    
	  int day1[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
		if(year%100!=0&&year%4==0||year%400==0){
		day1[2]=29;
		}
		return day1[month];
}
int get_length_year(int year){
    if(year%100!=0&&year%4==0||year%400==0){
		return 366;
		}
		return 365;
}
int get_week_between(int year,int month,int day){
    int day1[13]={0,31,28,31,30,31,30,31,31,30,31,30,31},temp;
    int day2[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
		RTC_DateTypeDef RTC_DateStruct;	
		int week,lower=1;
	  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		RTC_DateStruct.RTC_Year+=2000;
		week=0;
		
		
		if(year>RTC_DateStruct.RTC_Year||(year==RTC_DateStruct.RTC_Year&&month>RTC_DateStruct.RTC_Month)||(year==RTC_DateStruct.RTC_Year&&month==RTC_DateStruct.RTC_Month&&day>RTC_DateStruct.RTC_Date)){
		  temp=year;
			year=RTC_DateStruct.RTC_Year;
			RTC_DateStruct.RTC_Year=temp;
			temp=month;
			month=RTC_DateStruct.RTC_Month;
			RTC_DateStruct.RTC_Month=temp;
			temp=day;
			day=RTC_DateStruct.RTC_Date;
			RTC_DateStruct.RTC_Date=temp;
			lower=!lower;
		}
		if(year%100!=0&&year%4==0||year%400==0){
		day1[2]=29;
		
		}
		if( RTC_DateStruct.RTC_Year%100!=0&& RTC_DateStruct.RTC_Year%4==0||RTC_DateStruct.RTC_Year%400==0){
		day2[2]=29;
	
		}

		if(year<RTC_DateStruct.RTC_Year){
		while(year<RTC_DateStruct.RTC_Year){
		for(;month<13;month++){
		   week+=day1[month];
		}
		month=1;
		year++;
		}
		
		for(;month<RTC_DateStruct.RTC_Month;month++){
		   week+=day2[month];
		}
		week+=RTC_DateStruct.RTC_Date-day;
		}
		else if(month<RTC_DateStruct.RTC_Month){
			for(;month<RTC_DateStruct.RTC_Month;month++){
		   week+=day2[month];
		  }
			week+=RTC_DateStruct.RTC_Date-day;
		}
		else if(day<RTC_DateStruct.RTC_Date){
		  week+=RTC_DateStruct.RTC_Date-day;
		}
		week%=7;
		if(lower){
		week=RTC_DateStruct.RTC_WeekDay-week;
		if(week<1){
		 week+=7;
		}
		}
		else{
		week=RTC_DateStruct.RTC_WeekDay+week;
		if(week>7){
		week-=7;
		}
		}
		return week;
}
void set_time_click(void){   
	    int beginx=10,beginy=10,height=100,width=100,betweeny=10,i=0;
      RTC_TimeTypeDef RTC_TimeStruct;
	    
	    int year,month,date,hour,minute,week;
      RTC_DateTypeDef RTC_DateStruct;	
	    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	    year=RTC_DateStruct.RTC_Year+2000;
	    month=RTC_DateStruct.RTC_Month;
	    date=RTC_DateStruct.RTC_Date;
	    week=RTC_DateStruct.RTC_WeekDay;    
	
	    LCD_Clear(bk_color);
	    POINT_COLOR=THEME_TEXT;
	    BACK_COLOR=bk_color;
      RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	    hour=RTC_TimeStruct.RTC_Hours;
	    minute=RTC_TimeStruct.RTC_Minutes;
	
	    set_time_click_buttons();
	    
	    LCD_ShowString(beginx+width+10,beginy+height/2,100,24,24,"year:");
	    LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,year,4,24,24);		
			
	    LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"month:");
		  LCD_ShowxNum(beginx+width+10+12*6,beginy+height/2+(height+betweeny)*i,month,4,24,24);		
			
			LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"date:");
		  LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*i,date,4,24,24);		

			
			LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"hour:");
			LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*i,hour,2,24,24);		
			
			LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"minute:");
			LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*i,minute,2,24,24);	
			
			
			
	    while(now==SETTIMECLICK){
			  set_time_click_click();
				switch(add){
					case 1:year++;if(year<0)year=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,year,4,24,24);break;
					case 2:month++;if(month<1)month=12;if(month>12)month=1;LCD_ShowxNum(beginx+width+10+12*6,beginy+height/2+(height+betweeny),month,4,24,24);break;
					case 3:date++;if(date<1)date=get_max_day(year,month);if(date>get_max_day(year,month))date=1;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*2,date,4,24,24);break;
				  case 4:hour++; if(hour<0)hour=23;if(hour>23)hour=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*3,hour,2,24,24);break;	
				  case 5:minute++;if(minute<0)minute=59;if(minute>59)minute=0;LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*4,minute,2,24,24);break;
				}
				add=0;
				switch(sub){
					case 1:year--;if(year<0)year=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,year,4,24,24);
					case 2:month--;if(month<1)month=12;if(month>12)month=1;LCD_ShowxNum(beginx+width+10+12*6,beginy+height/2+(height+betweeny),month,4,24,24);
					case 3:date--;if(date<1)date=get_max_day(year,month);if(date>get_max_day(year,month))date=1;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*2,date,4,24,24);break;
				  case 4:hour--; if(hour<0)hour=23;if(hour>23)hour=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*3,hour,2,24,24);break;	
				  case 5:minute--;if(minute<0)minute=59;if(minute>59)minute=0;LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*4,minute,2,24,24);break;
				}
				sub=0;

				key_handle();
				delay_ms(100);
			}
			if(confirm==1){
			week=get_week_between(year,month,date);
			RTC_Set_Date(year-2000,month,date,week);
			RTC_Set_Time(hour,minute,0,RTC_H12_AM);
			}
	    
}
void set_time_str(char*str){
     int hour=0,minute=0;
	   int i=0;
	if(str[0]=='0'&&str[1]!=':')
		return;
	while(str[i]!=':'){
	if(str[i]<'0'||str[i]>'9')
		return;
	hour*=10;
	hour+=str[i]-'0';
	i++;
	}
	i++;
	while(str[i]!='\0'){
	if(str[i]<'0'||str[i]>'9')
		return;
	minute*=10;
	minute+=str[i]-'0';
	i++;
	}
	if(hour>23||minute>59){
	return;
	}
	RTC_Set_Time(hour,minute,0,RTC_H12_AM);
}
void set_time_usart_buttons(void){
   int beginx=10,beginy=10,height=100,width=100,between=200,betweeny=10+height;
   confirm_button.x=beginx;
   confirm_button.y=beginy+betweeny*6;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_ACCENT;
	 strcpy(confirm_button.str,"confirm");
	 draw_button(confirm_button);
	
	
	 cancel_button.x= confirm_button.x1+between;
   cancel_button.y= confirm_button.y;
   cancel_button.x1=cancel_button.x+width;
   cancel_button.y1=cancel_button.y+height;
   cancel_button.color=THEME_DANGER;
	 strcpy(cancel_button.str,"cancel");
	 draw_button(cancel_button);
}
void set_time_usart_click(void){
	
if(tp_dev.scan(0)){
if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
	confirm=1;
  now=CLOCK;
}
else if(tp_dev.x[0]>cancel_button.x&&tp_dev.x[0]<cancel_button.x1&&tp_dev.y[0]>cancel_button.y&&tp_dev.y[0]<cancel_button.y1)
{
  confirm=0;
  now=CLOCK;
}
	 }
}
void set_time_usart(void){
     
	    //u8 show3[50];
     
	     usart_data[0]='\0';
       LCD_Clear(bk_color);
	 
	     confirm=1;
	     usart_data[0]='\0';
	     POINT_COLOR=THEME_TEXT;
	     BACK_COLOR=bk_color;
	     LCD_ShowString(40,300,210,24,24,(u8*)"input:");
	     set_time_usart_buttons();
	     while(now==SETTIMEUSART){	 
	  	 if(input==1){
			 LCD_Fill(40+12*6,300,40+12*11,320, bk_color);
			 LCD_ShowString(40+12*6,300,210,24,24,(u8*)usart_data);
			 input=0;
			 }
			 set_time_usart_click();
			 key_handle();
			 delay_ms(100);
			 }
			if(confirm==1){
			  set_time_str(usart_data);
			}
			usart_data[0]='\0';
}
// draw a white card/panel with a light border, used as a backdrop for text areas
void draw_panel(int x,int y,int x1,int y1){
      LCD_Fill(x,y,x1,y1,THEME_PANEL);
      POINT_COLOR=THEME_EDGE;
      LCD_DrawLine(x,y,x1,y);
      LCD_DrawLine(x,y1,x1,y1);
      LCD_DrawLine(x,y,x,y1);
      LCD_DrawLine(x1,y,x1,y1);
}
void draw_button(struct button b){
      int len=strlen(b.str);
      int tw=len*12;                 // 24px font => 12px per char
      int tx=b.x+((b.x1-b.x)-tw)/2;  // center text horizontally
      int ty=(b.y+b.y1)/2-12;
      int hi=b.y+(b.y1-b.y)/3;       // top highlight band height
      if(tx<b.x+4)tx=b.x+4;
      // body
      LCD_Fill(b.x,b.y,b.x1,b.y1,b.color);
      // top highlight band for a subtle 3D look
      LCD_Fill(b.x+1,b.y+1,b.x1-1,hi,THEME_BTN_HI);
      // border
      POINT_COLOR=THEME_EDGE;
      LCD_DrawLine(b.x,b.y,b.x1,b.y);
      LCD_DrawLine(b.x,b.y,b.x,b.y1);
      // bottom/right shadow line
      POINT_COLOR=THEME_SHADOW;
      LCD_DrawLine(b.x,b.y1,b.x1,b.y1);
      LCD_DrawLine(b.x1,b.y,b.x1,b.y1);
      // solid strip behind the label so it sits on a uniform color
      LCD_Fill(b.x+1,ty,b.x1-1,ty+24,b.color);
      // centered label
      POINT_COLOR=THEME_TEXT_LT;
      BACK_COLOR=b.color;
      LCD_ShowString(tx,ty,(b.x1-b.x),24,24,(u8*)b.str);
      // restore default text colors so later LCD_ShowString/ShowxNum calls
      // (e.g. the year/hour numbers on setting pages) are not left white-on-bg
      POINT_COLOR=THEME_TEXT;
      BACK_COLOR=bk_color;
}
void draw_between(struct button left,struct button right,int color){
      LCD_Fill(left.x1,left.y,right.x,right.y1,color);
	    POINT_COLOR=THEME_EDGE;
	
	    LCD_DrawLine(left.x1,left.y,right.x,right.y);
	    LCD_DrawLine(left.x1,left.y,left.x1,left.y1);
	    
	    LCD_DrawLine(left.x1,left.y1,right.x,right.y1);
	    LCD_DrawLine(right.x1,right.y,right.x1,right.y1);
	
	    BACK_COLOR=color;
	    BACK_COLOR=bk_color;
}
void clock_buttons_click(void){
if(tp_dev.scan(0)){
if(tp_dev.x[0]>set_time_button.x&&tp_dev.x[0]<set_time_button.x1&&tp_dev.y[0]>set_time_button.y&&tp_dev.y[0]<set_time_button.y1)
{
  now=SETTIME;
}
else if(tp_dev.x[0]>set_clock_button.x&&tp_dev.x[0]<set_clock_button.x1&&tp_dev.y[0]>set_clock_button.y&&tp_dev.y[0]<set_clock_button.y1){
	now=ALARMCLOCK;
}
else if(tp_dev.x[0]>count_button.x&&tp_dev.x[0]<count_button.x1&&tp_dev.y[0]>count_button.y&&tp_dev.y[0]<count_button.y1){
	now=COUNT;
}
else if(tp_dev.x[0]>countdown_button.x&&tp_dev.x[0]<countdown_button.x1&&tp_dev.y[0]>countdown_button.y&&tp_dev.y[0]<countdown_button.y1){
	now=SETCOUNTDOWN;
}
}
}
void clock_buttons(void){
	 int beginx=0,beginy=660,height=100,width=120;


   set_time_button.x=beginx;
	 set_time_button.y=beginy;
   set_time_button.x1=beginx+width;
   set_time_button.y1=beginy+height;
   set_time_button.color=THEME_BTN;
	 strcpy(set_time_button.str,"set-time");
	 draw_button(set_time_button);

	 set_clock_button.x=beginx+width;
	 set_clock_button.y=beginy;
   set_clock_button.x1=set_clock_button.x+width;
   set_clock_button.y1=set_clock_button.y+height;
   set_clock_button.color=THEME_BTN;
	 strcpy(set_clock_button.str,"alarm");
	 draw_button(set_clock_button);


	 count_button.x=beginx+width*2;
	 count_button.y=beginy;
   count_button.x1= count_button.x+width;
   count_button.y1= count_button.y+height;
   count_button.color=THEME_BTN;
	 strcpy(count_button.str,"count");
	 draw_button(count_button);

	 countdown_button.x=beginx+width*3;
	 countdown_button.y=beginy;
   countdown_button.x1= countdown_button.x+width;
   countdown_button.y1= countdown_button.y+height;
   countdown_button.color=THEME_BTN;
	 strcpy(countdown_button.str,"cntdown");
	 draw_button(countdown_button);
}
void show_clock(void){
	int last_sec=-1,temp_cnt=0;
	RTC_TimeTypeDef t;
	back();
  clock();
	draw_info_frame();
	clock_buttons();
	while(now==CLOCK)
	{
			//按需刷新:秒变化才重画时间/日期/指针,温度每5秒一次。
			//这样秒针每秒精确跳一格,不再整循环耗时拖成"一帧一帧卡"。
			RTC_GetTime(RTC_Format_BIN,&t);
			if(t.RTC_Seconds!=last_sec){
				last_sec=t.RTC_Seconds;
				show_date();
				show_time();
				draw_time();
				hour_chime();           // 整点报时
				if(--temp_cnt<=0){ show_temprature(); temp_cnt=5; }
			}
			clock_buttons_click();    //触摸每轮都查,响应跟手
			key_handle();
		  delay_ms(20);
	}
}
void set_alarm_clock_menu_buttons(void){
   int beginx=100,beginy=100,height=200,width=200;
	
   add_button.x=beginx;
	 add_button.y=beginy;
   add_button.x1=beginx+width;
   add_button.y1=beginy+height;
   add_button.color=THEME_BTN;
	 strcpy(add_button.str,"add alarm clock");
	 draw_button(add_button);
	
	 drop_button.x=beginx;
	 drop_button.y=beginy+height*2;
   drop_button.x1=drop_button.x+width;
   drop_button.y1=drop_button.y+height;
   drop_button.color=THEME_BTN;
	 strcpy(drop_button.str,"drop alarm clock");
	 draw_button(drop_button);
}
void set_alarm_clock_menu_click(void){
if(tp_dev.scan(0)){
if(tp_dev.x[0]>add_button.x&&tp_dev.x[0]<add_button.x1&&tp_dev.y[0]>add_button.y&&tp_dev.y[0]<add_button.y1)
{
  now=ADDALARMCLOCK;
}
else if(tp_dev.x[0]>drop_button.x&&tp_dev.x[0]<drop_button.x1&&tp_dev.y[0]>drop_button.y&&tp_dev.y[0]<drop_button.y1)
{
  now=DROPALARMCLOCK;
}
}
}
void set_alarm_clock_menu(void){
	  now=ALARMCLOCK;
	  back();
	  set_alarm_clock_menu_buttons();
	  while(now==ALARMCLOCK){
		set_alarm_clock_menu_click();
		key_handle();
		delay_ms(100);
		};
}
void add_alarm_clock_buttons(void){
 int beginx=10,beginy=10,height=100,width=100,between=200,betweeny=10+height;
   
	
   add_button1.x=beginx;
   add_button1.y=beginy;
   add_button1.x1=beginx+width;
   add_button1.y1=beginy+height;
   add_button1.color=THEME_BTN;
	 strcpy(add_button1.str,"+");
	 draw_button(add_button1);
	
	 sub_button1.x= add_button1.x1+between;
   sub_button1.y= add_button1.y;
   sub_button1.x1=sub_button1.x+width;
   sub_button1.y1=sub_button1.y+height;
   sub_button1.color=THEME_BTN;
	 strcpy(sub_button1.str,"-");
	 draw_button(sub_button1);
	
	 add_button2.x=beginx;
   add_button2.y=add_button1.y1+betweeny;
   add_button2.x1=add_button2.x+width;
   add_button2.y1=add_button2.y+height;
   add_button2.color=THEME_BTN;
	 strcpy(add_button2.str,"+");
	 draw_button(add_button2);
	 
	 sub_button2.x= add_button1.x1+between;
   sub_button2.y= add_button2.y;
   sub_button2.x1=sub_button2.x+width;
   sub_button2.y1=sub_button2.y+height;
   sub_button2.color=THEME_BTN;
	 strcpy(sub_button2.str,"-");
	 draw_button(sub_button2);
	
	 add_button3.x=beginx;
   add_button3.y=add_button2.y1+betweeny;
   add_button3.x1=add_button3.x+width;
   add_button3.y1=add_button3.y+height;
   add_button3.color=THEME_BTN;
	 strcpy(add_button3.str,"+");
	 draw_button(add_button3);
	 
	 sub_button3.x= add_button3.x1+between;
   sub_button3.y= add_button3.y;
   sub_button3.x1=sub_button3.x+width;
   sub_button3.y1=sub_button3.y+height;
   sub_button3.color=THEME_BTN;
	 strcpy(sub_button3.str,"-");
	 draw_button(sub_button3);
	 
	 confirm_button.x=beginx;
   confirm_button.y=beginy+betweeny*6;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_ACCENT;
	 strcpy(confirm_button.str,"confirm");
	 draw_button(confirm_button);
	
	
	 cancel_button.x= confirm_button.x1+between;
   cancel_button.y= confirm_button.y;
   cancel_button.x1=cancel_button.x+width;
   cancel_button.y1=cancel_button.y+height;
   cancel_button.color=THEME_DANGER;
	 strcpy(cancel_button.str,"cancel");
	 draw_button(cancel_button);
	
}
void add_alarm_clock_click(void){
     if(tp_dev.scan(0)){
if(tp_dev.x[0]>add_button1.x&&tp_dev.x[0]<add_button1.x1&&tp_dev.y[0]>add_button1.y&&tp_dev.y[0]<add_button1.y1)
{
  add=1;
}
else if(tp_dev.x[0]>add_button2.x&&tp_dev.x[0]<add_button2.x1&&tp_dev.y[0]>add_button2.y&&tp_dev.y[0]<add_button2.y1)
{
  add=2;
}
else if(tp_dev.x[0]>add_button3.x&&tp_dev.x[0]<add_button3.x1&&tp_dev.y[0]>add_button3.y&&tp_dev.y[0]<add_button3.y1)
{
  add=3;
}
else if(tp_dev.x[0]>sub_button1.x&&tp_dev.x[0]<sub_button1.x1&&tp_dev.y[0]>sub_button1.y&&tp_dev.y[0]<sub_button1.y1)
{
  sub=1;
}
else if(tp_dev.x[0]>sub_button2.x&&tp_dev.x[0]<sub_button2.x1&&tp_dev.y[0]>sub_button2.y&&tp_dev.y[0]<sub_button2.y1)
{
  sub=2;
}
else if(tp_dev.x[0]>sub_button3.x&&tp_dev.x[0]<sub_button3.x1&&tp_dev.y[0]>sub_button3.y&&tp_dev.y[0]<sub_button3.y1)
{
  sub=3;
}
else if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
	confirm=1;
  now=CLOCK;
}
else if(tp_dev.x[0]>cancel_button.x&&tp_dev.x[0]<cancel_button.x1&&tp_dev.y[0]>cancel_button.y&&tp_dev.y[0]<cancel_button.y1)
{
  confirm=0;
  now=CLOCK;
}
}
}
void add_alarm_clock_gui(void){
      RTC_TimeTypeDef RTC_TimeStruct;
	    int week,hour,minute;
	     int beginx=10,beginy=10,height=100,width=100,betweeny=10;	   
	    LCD_Clear(bk_color);
	   
	    POINT_COLOR=THEME_TEXT;
	    BACK_COLOR=bk_color;
      RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
	    hour=RTC_TimeStruct.RTC_Hours;
	    minute=RTC_TimeStruct.RTC_Minutes;
			week=0;
	    add_alarm_clock_buttons();
			LCD_ShowString(beginx+width+10,beginy+height/2,100,24,24,"week:");
	    LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,week,4,24,24);		
			
	    LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*2,100,24,24,"hour:");
		  LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*2,hour,4,24,24);		
			
			LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*4,100,24,24,"minute:");
		  LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*4,minute,4,24,24);		
			
	    while(now==ADDALARMCLOCK){	
			add_alarm_clock_click();
				switch(add){
	        case 1:week++;if(week<0)week=7;if(week>7)week=0;  LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,week,4,24,24);break;				
				  case 2:hour++; if(hour<0)hour=23;if(hour>23)hour=0; LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*2,hour,4,24,24);break;	
				  case 3:minute++;if(minute<0)minute=59;if(minute>59)minute=0; LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*4,minute,4,24,24);break;
				}
				add=0;
				switch(sub){
	        case 1:week--;if(week<0)week=7;if(week>7)week=0;  LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,week,4,24,24);break;				
				  case 2:hour--; if(hour<0)hour=23;if(hour>23)hour=0; LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2+(height+betweeny)*2,hour,4,24,24);break;	
				  case 3:minute--;if(minute<0)minute=59;if(minute>59)minute=0; LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*4,minute,4,24,24);break;
				}
				sub=0;
				key_handle();
				delay_ms(100);
	    }
			if(confirm==1){
			 struct time temp;
			 temp.hour=hour;
			 temp.minute=minute;
			 temp.week=week;
		   add_alarm_clock(temp);
			}
}
void drop_alarm_clock_buttons(void){
   int i=0;   
	 int beginx=10,beginy=10,height=100,width=400,between=height+10;
	 u8 buf[30];
	 for(i=0;i<my_alarm_clock.point;i++){
	 alarm_clock_list[i].x=beginx;
	 alarm_clock_list[i].y=beginy+between*i;
   alarm_clock_list[i].x1=beginx+width;
	 alarm_clock_list[i].y1=alarm_clock_list[i].y+height;
	 sprintf((char*)buf,"week:%02d %02d:%02d",my_alarm_clock.clock[i].week,my_alarm_clock.clock[i].hour,my_alarm_clock.clock[i].minute); 
	 strcpy( alarm_clock_list[i].str,(char*)buf);
	 alarm_clock_list[i].color=THEME_BTN;
	 draw_button( alarm_clock_list[i]);
	 }
	 if(my_alarm_clock.point!=0){
	 i=my_alarm_clock.point;
	 confirm_button.x=beginx;
   confirm_button.y=beginy+between*i;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_ACCENT;
	 strcpy(confirm_button.str,"confirm");
	 draw_button(confirm_button);
	 }
	 i++;
	 cancel_button.x= beginx;
   cancel_button.y= beginy+between*i;
   cancel_button.x1=cancel_button.x+width;
   cancel_button.y1=cancel_button.y+height;
   cancel_button.color=THEME_DANGER;
	 strcpy(cancel_button.str,"cancel");
	 draw_button(cancel_button);
}
void drop_alarm_clock_click(void){
int i=0;
if(tp_dev.scan(0)){
if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
	confirm=1;
  now=CLOCK;
}
else if(tp_dev.x[0]>cancel_button.x&&tp_dev.x[0]<cancel_button.x1&&tp_dev.y[0]>cancel_button.y&&tp_dev.y[0]<cancel_button.y1)
{
  confirm=0;
  now=CLOCK;
}

for(i=0;i<my_alarm_clock.point;i++){
if(tp_dev.x[0]>alarm_clock_list[i].x&&tp_dev.x[0]<alarm_clock_list[i].x1&&tp_dev.y[0]>alarm_clock_list[i].y&&tp_dev.y[0]<alarm_clock_list[i].y1)
{
	last_dropi=dropi;
  dropi=i;
	alarm_clock_list[last_dropi].color=THEME_BTN;
	draw_button(alarm_clock_list[last_dropi]);
	alarm_clock_list[dropi].color= THEME_BTN_SEL;
	draw_button( alarm_clock_list[dropi]);
	break;
}
}
}
}
void drop_alarm_clock_gui(void){
	    dropi=0;    
	    LCD_Clear(bk_color);
	    drop_alarm_clock_buttons();
	    alarm_clock_list[dropi].color= THEME_BTN_SEL;
	    draw_button( alarm_clock_list[dropi]);
	    while(now==DROPALARMCLOCK){
      drop_alarm_clock_click();
			key_handle();
			delay_ms(100);
	    }
			if(confirm==1){
			if(my_alarm_clock.point>0){
			drop_alarm_clock(dropi);
			}
			}
}

void set_time_buttons(void){
   int beginx=100,beginy=100,height=200,width=200;
	
   set_time_port_button.x=beginx;
	 set_time_port_button.y=beginy;
   set_time_port_button.x1=beginx+width;
   set_time_port_button.y1=beginy+height;
   set_time_port_button.color=THEME_BTN;
	 strcpy(set_time_port_button.str,"port");
	 draw_button(set_time_port_button);
	
	 set_time_click_button.x=beginx;
	 set_time_click_button.y=beginy+height*2;
   set_time_click_button.x1=set_time_click_button.x+width;
   set_time_click_button.y1=set_time_click_button.y+height;
   set_time_click_button.color=THEME_BTN;
	 strcpy(set_time_click_button.str,"click");
	 draw_button(set_time_click_button);
}
void set_time_buttons_click(void){
if(tp_dev.scan(0)){
if(tp_dev.x[0]>set_time_port_button.x&&tp_dev.x[0]<set_time_port_button.x1&&tp_dev.y[0]>set_time_port_button.y&&tp_dev.y[0]<set_time_port_button.y1)
{
  now=SETTIMEUSART;
}
else if(tp_dev.x[0]>set_time_click_button.x&&tp_dev.x[0]<set_time_click_button.x1&&tp_dev.y[0]>set_time_click_button.y&&tp_dev.y[0]<set_time_click_button.y1){
	now=SETTIMECLICK;
}
}
}
void set_time_menu(void){
	   back();
	   set_time_buttons();
	   while(now==SETTIME){
		 set_time_buttons_click();
		 key_handle();
		 }
}
void count_click(void){
if(tp_dev.scan(0)){
if(tp_dev.x[0]>continue_button.x&&tp_dev.x[0]<continue_button.x1&&tp_dev.y[0]>continue_button.y&&tp_dev.y[0]<continue_button.y1)
{ 
	 if(stop==1){
	 stop=0; 
	 continue_=1;
	 }
}
else if(tp_dev.x[0]>stop_button.x&&tp_dev.x[0]<stop_button.x1&&tp_dev.y[0]>stop_button.y&&tp_dev.y[0]<stop_button.y1)
{
	 stop=1;
}
else if(tp_dev.x[0]>lap_button.x&&tp_dev.x[0]<lap_button.x1&&tp_dev.y[0]>lap_button.y&&tp_dev.y[0]<lap_button.y1)
{
	 lap_flag=1;
}
else if(tp_dev.x[0]>restart_button.x&&tp_dev.x[0]<restart_button.x1&&tp_dev.y[0]>restart_button.y&&tp_dev.y[0]<restart_button.y1)
{
   restart=1;
	 stop=0;
}
else if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
   now=CLOCK;
}
}
}
//�����������һȦ�����ʾ�ڵ�lap_count��
void show_one_lap(int idx){
	u8 lbuf[24];
	if(idx<0||idx>=5)return;
	sprintf((char*)lbuf,"L%d %02d:%02d:%02d",idx+1,lap_times[idx][0],lap_times[idx][1],lap_times[idx][2]);
	POINT_COLOR=THEME_TEXT;
	BACK_COLOR=bk_color;
	LCD_ShowString(330,480+idx*30,150,24,24,lbuf);
}
//�����ǰ��Ȧ��¼��ֻ��С��5Ȧʱ��¼
void record_lap(int h,int m,int s){
	if(lap_count>=5)return;
	lap_times[lap_count][0]=h;
	lap_times[lap_count][1]=m;
	lap_times[lap_count][2]=s;
	show_one_lap(lap_count);
	lap_count++;
}
//�����Ȧ��¼����ʾ��
void clear_laps(void){
	lap_count=0;
	LCD_Fill(330,470,479,470+5*30,bk_color);
}
void count_buttons(void){
	 int beginx=10,beginy=10,height=50,width=300,between=height+10;

   continue_button.x=beginx;
   continue_button.y=beginy+between*8;
   continue_button.x1= continue_button.x+width;
   continue_button.y1=continue_button.y+height;
   continue_button.color=THEME_ACCENT;
	 strcpy(continue_button.str,"continue");
	 draw_button( continue_button);

	 stop_button.x=continue_button.x;
   stop_button.y=continue_button.y+between;
   stop_button.x1=stop_button.x+width;
   stop_button.y1=stop_button.y+height;
   stop_button.color=THEME_BTN;
	 strcpy(stop_button.str,"stop");
	 draw_button(stop_button);

	 lap_button.x=continue_button.x;
   lap_button.y=stop_button.y+between;
   lap_button.x1=lap_button.x+width;
   lap_button.y1=lap_button.y+height;
   lap_button.color=THEME_BTN;
	 strcpy(lap_button.str,"lap");
	 draw_button(lap_button);

	 restart_button.x=continue_button.x;
   restart_button.y=lap_button.y+between;
   restart_button.x1=restart_button.x+width;
   restart_button.y1=restart_button.y+height;
   restart_button.color=THEME_BTN;
	 strcpy(restart_button.str,"restart");
	 draw_button(restart_button);

	 confirm_button.x=continue_button.x;
   confirm_button.y=restart_button.y+between;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_DANGER;
	 strcpy( confirm_button.str,"exit");
	 draw_button(confirm_button);
}
void count_menu(void){
	   u8 tbuf[20];
     int hour=0,minute=0,seconds=0; 
	   int midx=250,midy=250;
	   u16 lineColor=THEME_SEC;
	   int length=100,x,y,x1,y1;
	   RTC_TimeTypeDef Start_RTC_TimeStruct,Now_RTC_TimeStruct;
     
	   back();
     clock();
	   count_buttons();
	   POINT_COLOR=THEME_TEXT;
	   BACK_COLOR=bk_color;
	   sprintf((char*)tbuf,"Time:%02d:%02d:%02d",hour,minute,seconds); 
		 LCD_ShowString(160,480,210,24,24,tbuf);	
	   x=midx+(length)*(cos(PI*2*seconds/60-PI/2));
		 y=midy+(length)*(sin(PI*2*seconds/60-PI/2));
		 x1=midx;
		 y1=midy;
	   POINT_COLOR=lineColor;
		 LCD_DrawLine(x,y,x1,y1);
	   restart=0;
	   continue_=0;
	   stop=1;
	   lap_flag=0;
	   clear_laps();
	   POINT_COLOR=THEME_SUB;
	   BACK_COLOR=bk_color;
	   LCD_ShowString(10,720,470,16,16,(u8*)"K0:run/pause K1:lap K2:reset WK:exit");
	   while(now==COUNT){
		 count_click();
		 key_handle();

		 if(restart==1){
		   RTC_GetTime(RTC_Format_BIN,&Start_RTC_TimeStruct);
			 restart=0;
			 clear_laps();
		 }
		 if(continue_==1){
			 RTC_GetTime(RTC_Format_BIN,&Now_RTC_TimeStruct);
		   Start_RTC_TimeStruct.RTC_Hours=Now_RTC_TimeStruct.RTC_Hours-hour;
			 Start_RTC_TimeStruct.RTC_Minutes=Now_RTC_TimeStruct.RTC_Minutes-minute;
			 Start_RTC_TimeStruct.RTC_Seconds=Now_RTC_TimeStruct.RTC_Seconds-seconds;
			 continue_=0;
		 }
     if(stop==0){
     RTC_GetTime(RTC_Format_BIN,&Now_RTC_TimeStruct);
		 hour=Now_RTC_TimeStruct.RTC_Hours-Start_RTC_TimeStruct.RTC_Hours;
	   minute=Now_RTC_TimeStruct.RTC_Minutes-Start_RTC_TimeStruct.RTC_Minutes;
		 seconds=Now_RTC_TimeStruct.RTC_Seconds-Start_RTC_TimeStruct.RTC_Seconds;
     if(seconds<0){
		 seconds+=60;
		 minute--;
		 }
     if(minute<0){
		 minute+=60;
		 hour--;
		 }
		 clear_time_area();
		 POINT_COLOR=THEME_TEXT;
		 BACK_COLOR=bk_color;
		 sprintf((char*)tbuf,"Time:%02d:%02d:%02d",hour,minute,seconds); 
		 LCD_ShowString(160,480,210,24,24,tbuf);	
	   x=midx+(length)*(cos(PI*2*seconds/60-PI/2));
		 y=midy+(length)*(sin(PI*2*seconds/60-PI/2));
		 x1=midx;
		 y1=midy;
	   POINT_COLOR=lineColor;
		 LCD_DrawLine(x,y,x1,y1);
		 }

		 if(lap_flag==1){
		   if(stop==0)record_lap(hour,minute,seconds);
		   lap_flag=0;
		 }

		 delay_ms(100);
		 }
}
void count_down_buttons(void){
   int beginx=10,beginy=10,height=50,width=400,between=height+10;
	
   continue_button.x=beginx;
   continue_button.y=beginy+between*9;
   continue_button.x1= continue_button.x+width;
   continue_button.y1=continue_button.y+height;
   continue_button.color=THEME_ACCENT;
	 strcpy(continue_button.str,"continue");
	 draw_button( continue_button);

	 stop_button.x=continue_button.x;
   stop_button.y=continue_button.y+between;
   stop_button.x1=stop_button.x+width;
   stop_button.y1=stop_button.y+height;
   stop_button.color=THEME_BTN;
	 strcpy(stop_button.str,"stop");
	 draw_button(stop_button);

	 confirm_button.x=continue_button.x;
   confirm_button.y=stop_button.y+between;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_DANGER;
	 strcpy( confirm_button.str,"exit");
	 draw_button(confirm_button);
}
void count_down_click(void){
if(tp_dev.scan(0)){
if(tp_dev.x[0]>continue_button.x&&tp_dev.x[0]<continue_button.x1&&tp_dev.y[0]>continue_button.y&&tp_dev.y[0]<continue_button.y1)
{ 
	 if(stop==1){
	 stop=0; 
	 continue_=1;
	 }
}
else if(tp_dev.x[0]>stop_button.x&&tp_dev.x[0]<stop_button.x1&&tp_dev.y[0]>stop_button.y&&tp_dev.y[0]<stop_button.y1)
{
	 stop=1;
}
else if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
   now=CLOCK;
}
} 
}
void count_down(void){
     u8 tbuf[20];
     int hour=hour_count_down,minute=minute_count_down,seconds=second_count_down; 
	   int midx=250,midy=250;
	   u16 lineColor=THEME_SEC;
	   int length=100,x,y,x1,y1;
	   RTC_TimeTypeDef Start_RTC_TimeStruct,Now_RTC_TimeStruct;
     
	   back();
     clock();
	   count_down_buttons();
	   POINT_COLOR=THEME_TEXT;
	   BACK_COLOR=bk_color;
	   sprintf((char*)tbuf,"Time:%02d:%02d:%02d",hour,minute,seconds); 
		 LCD_ShowString(160,480,210,24,24,tbuf);	
	   x=midx+(length)*(cos(PI*2*seconds/60-PI/2));
		 y=midy+(length)*(sin(PI*2*seconds/60-PI/2));
		 x1=midx;
		 y1=midy;
	   POINT_COLOR=lineColor;
		 LCD_DrawLine(x,y,x1,y1);
	   restart=0;
	   continue_=0;
	   stop=1;
	   POINT_COLOR=THEME_SUB;
	   BACK_COLOR=bk_color;
	   LCD_ShowString(10,720,470,16,16,(u8*)"K0:run/pause  WK:exit");
	   while(now==COUNTDOWN){
		 count_down_click();
		 key_handle();
			if(hour==0&&minute==0&&seconds==0){
			now=CLOCK;
			}
		 if(continue_==1){
			 RTC_GetTime(RTC_Format_BIN,&Now_RTC_TimeStruct);
		   Start_RTC_TimeStruct.RTC_Hours=Now_RTC_TimeStruct.RTC_Hours-hour;
			 Start_RTC_TimeStruct.RTC_Minutes=Now_RTC_TimeStruct.RTC_Minutes-minute;
			 Start_RTC_TimeStruct.RTC_Seconds=Now_RTC_TimeStruct.RTC_Seconds-seconds;
			 continue_=0;
		 }
     if(stop==0){
     RTC_GetTime(RTC_Format_BIN,&Now_RTC_TimeStruct);
		 hour=hour_count_down-(Now_RTC_TimeStruct.RTC_Hours-Start_RTC_TimeStruct.RTC_Hours);
	   minute=minute_count_down-(Now_RTC_TimeStruct.RTC_Minutes-Start_RTC_TimeStruct.RTC_Minutes);
		 seconds=second_count_down-(Now_RTC_TimeStruct.RTC_Seconds-Start_RTC_TimeStruct.RTC_Seconds);
		 if(seconds<0){
		 seconds+=60;
		 minute--;
		 }
     if(minute<0){
		 minute+=60;
		 hour--;
		 }
		 clear_time_area();
		 POINT_COLOR=THEME_TEXT;
		 BACK_COLOR=bk_color;
		 sprintf((char*)tbuf,"Time:%02d:%02d:%02d",hour,minute,seconds); 
		 LCD_ShowString(160,480,210,24,24,tbuf);	
	   x=midx+(length)*(cos(PI*2*seconds/60-PI/2));
		 y=midy+(length)*(sin(PI*2*seconds/60-PI/2));
		 x1=midx;
		 y1=midy;
	   POINT_COLOR=lineColor;
		 LCD_DrawLine(x,y,x1,y1);
		 }
		 delay_ms(100);
		 }
}
void set_count_down_buttons(void){
   int beginx=10,beginy=10,height=100,width=100,between=200,betweeny=10;
   add_button1.x=beginx;
   add_button1.y=beginy;
   add_button1.x1=beginx+width;
   add_button1.y1=beginy+height;
   add_button1.color=THEME_BTN;
	 strcpy(add_button1.str,"+");
	 draw_button(add_button1);
	
	 sub_button1.x= add_button1.x1+between;
   sub_button1.y= add_button1.y;
   sub_button1.x1=sub_button1.x+width;
   sub_button1.y1=sub_button1.y+height;
   sub_button1.color=THEME_BTN;
	 strcpy(sub_button1.str,"-");
	 draw_button(sub_button1);
	
	draw_between(add_button1,sub_button1,THEME_PANEL);
	 
	 
	
	 add_button2.x=beginx;
   add_button2.y=add_button1.y1+betweeny;
   add_button2.x1=add_button2.x+width;
   add_button2.y1=add_button2.y+height;
   add_button2.color=THEME_BTN;
	 strcpy(add_button2.str,"+");
	 draw_button(add_button2);
	 
	 sub_button2.x= add_button1.x1+between;
   sub_button2.y= add_button2.y;
   sub_button2.x1=sub_button2.x+width;
   sub_button2.y1=sub_button2.y+height;
   sub_button2.color=THEME_BTN;
	 strcpy(sub_button2.str,"-");
	 draw_button(sub_button2);
	 
	 draw_between(add_button2,sub_button2,THEME_PANEL);
	
	 add_button3.x=beginx;
   add_button3.y=add_button2.y1+betweeny;
   add_button3.x1=add_button3.x+width;
   add_button3.y1=add_button3.y+height;
   add_button3.color=THEME_BTN;
	 strcpy(add_button3.str,"+");
	 draw_button(add_button3);
	 
	 sub_button3.x= add_button3.x1+between;
   sub_button3.y= add_button3.y;
   sub_button3.x1=sub_button3.x+width;
   sub_button3.y1=sub_button3.y+height;
   sub_button3.color=THEME_BTN;
	 strcpy(sub_button3.str,"-");
	 draw_button(sub_button3);
	 
	 draw_between(add_button3,sub_button3,THEME_PANEL);
	 
	 betweeny=10+height;
	 confirm_button.x=beginx;
   confirm_button.y=beginy+betweeny*6;
   confirm_button.x1=confirm_button.x+width;
   confirm_button.y1=confirm_button.y+height;
   confirm_button.color=THEME_ACCENT;
	 strcpy(confirm_button.str,"confirm");
	 draw_button(confirm_button);
	
	 cancel_button.x= confirm_button.x1+between;
   cancel_button.y= confirm_button.y;
   cancel_button.x1=cancel_button.x+width;
   cancel_button.y1=cancel_button.y+height;
	 cancel_button.color=THEME_DANGER;
	 strcpy(cancel_button.str,"cancel");
	 draw_button(cancel_button);

	 //�Ҳມ��Ԥ�谴ť: 1min/3min/5min/10min/30min
	 {
	   int pi,px=360,pw=110,ph=60,pgap=10,py0=10;
	   char pbuf[8];
	   POINT_COLOR=THEME_TEXT;
	   BACK_COLOR=bk_color;
	   BACK_COLOR=bk_color;
	   LCD_ShowString(px,py0-2,pw,16,16,(u8*)"preset");
	   for(pi=0;pi<5;pi++){
	     preset_button[pi].x=px;
	     preset_button[pi].y=py0+20+(ph+pgap)*pi;
	     preset_button[pi].x1=px+pw;
	     preset_button[pi].y1=preset_button[pi].y+ph;
	     preset_button[pi].color=THEME_BTN;
	     sprintf(pbuf,"%dmin",preset_minutes[pi]);
	     strcpy(preset_button[pi].str,pbuf);
	     draw_button(preset_button[pi]);
	   }
	 }
}
void set_count_down_click(void){
 if(tp_dev.scan(0)){
if(tp_dev.x[0]>add_button1.x&&tp_dev.x[0]<add_button1.x1&&tp_dev.y[0]>add_button1.y&&tp_dev.y[0]<add_button1.y1)
{
  add=1;
}
else if(tp_dev.x[0]>add_button2.x&&tp_dev.x[0]<add_button2.x1&&tp_dev.y[0]>add_button2.y&&tp_dev.y[0]<add_button2.y1)
{
  add=2;
}
else if(tp_dev.x[0]>add_button3.x&&tp_dev.x[0]<add_button3.x1&&tp_dev.y[0]>add_button3.y&&tp_dev.y[0]<add_button3.y1)
{
  add=3;
}
else if(tp_dev.x[0]>sub_button1.x&&tp_dev.x[0]<sub_button1.x1&&tp_dev.y[0]>sub_button1.y&&tp_dev.y[0]<sub_button1.y1)
{
  sub=1;
}
else if(tp_dev.x[0]>sub_button2.x&&tp_dev.x[0]<sub_button2.x1&&tp_dev.y[0]>sub_button2.y&&tp_dev.y[0]<sub_button2.y1)
{
  sub=2;
}
else if(tp_dev.x[0]>sub_button3.x&&tp_dev.x[0]<sub_button3.x1&&tp_dev.y[0]>sub_button3.y&&tp_dev.y[0]<sub_button3.y1)
{
  sub=3;
}
else if(tp_dev.x[0]>preset_button[0].x&&tp_dev.x[0]<preset_button[0].x1&&tp_dev.y[0]>preset_button[0].y&&tp_dev.y[0]<preset_button[4].y1)
{
  int pi;
  for(pi=0;pi<5;pi++){
    if(tp_dev.y[0]>preset_button[pi].y&&tp_dev.y[0]<preset_button[pi].y1){
      preset_pick=pi;
      break;
    }
  }
}
else if(tp_dev.x[0]>confirm_button.x&&tp_dev.x[0]<confirm_button.x1&&tp_dev.y[0]>confirm_button.y&&tp_dev.y[0]<confirm_button.y1)
{
	confirm=1;
  now=COUNTDOWN;
}
else if(tp_dev.x[0]>cancel_button.x&&tp_dev.x[0]<cancel_button.x1&&tp_dev.y[0]>cancel_button.y&&tp_dev.y[0]<cancel_button.y1)
{
  confirm=0;
  now=CLOCK;
}
}
}
void set_count_down_menu(void){
     
	    int second=0,hour=0,minute=0;
	    int beginx=10,beginy=10,height=100,width=100,betweeny=10,i=0;
	    preset_pick=-1;
	    LCD_Clear(bk_color);
	    
	    POINT_COLOR=THEME_TEXT;
      BACK_COLOR=bk_color;
			LCD_ShowString(beginx+width+10,beginy+height/2,100,24,24,"hour:");
	    LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,hour,4,24,24);		
			
	    LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"minute:");
		  LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*i,minute,4,24,24);		
			
			LCD_ShowString(beginx+width+10,beginy+height/2+(height+betweeny)*++i,100,24,24,"second:");
		  LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*i,second,4,24,24);		
			 set_count_down_buttons();
	    
	    while(now==SETCOUNTDOWN){	
		 set_count_down_click();
				switch(add){
	        case 1:hour++; if(hour<0)hour=23;if(hour>23)hour=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,hour,4,24,24);	break;				
				  case 2:minute++;if(minute<0)minute=59;if(minute>59)minute=0;LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny),minute,4,24,24);	break;
				  case 3:second++;if(second<0)minute=59;if(second>59)second=0; LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*2,second,4,24,24);	break;
				}
				add=0;
				switch(sub){
				  case 1:hour--; if(hour<0)hour=23;if(hour>23)hour=0;LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,hour,4,24,24);	break;
				  case 2:minute--;if(minute<0)minute=59;if(minute>59)minute=0;LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny),minute,4,24,24);	break;
				  case 3:second--;if(second<0)minute=59;if(second>59)second=0; LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*2,second,4,24,24);	break;
				}
				sub=0;
				if(preset_pick>=0){
				  hour=preset_minutes[preset_pick]/60;
				  minute=preset_minutes[preset_pick]%60;
				  second=0;
				  LCD_ShowxNum(beginx+width+10+12*5,beginy+height/2,hour,4,24,24);
				  LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny),minute,4,24,24);
				  LCD_ShowxNum(beginx+width+10+12*7,beginy+height/2+(height+betweeny)*2,second,4,24,24);
				  preset_pick=-1;
				}
				key_handle();
				delay_ms(100);
	    }
			if(confirm==1){
			   hour_count_down=hour;
				 minute_count_down=minute;
				 second_count_down=second;
			}
}
void clock_with_buttons(void){
	int last_sec=-1,temp_cnt=0;
	RTC_TimeTypeDef t;
  back();
  clock();
	draw_info_frame();
	clock_buttons();
	while(now==CLOCKBUTTONS)
	{
			//按需刷新,同 show_clock
			RTC_GetTime(RTC_Format_BIN,&t);
			if(t.RTC_Seconds!=last_sec){
				last_sec=t.RTC_Seconds;
				show_date();
				show_time();
				draw_time();
				hour_chime();           // 整点报时
				if(--temp_cnt<=0){ show_temprature(); temp_cnt=5; }
			}
		  clock_buttons_click();
		  key_handle();
		  delay_ms(20);
	}
}
void key_handle(void){
	u8 k=KEY_Scan(0);
	if(k==0)return;
	switch(now){
		case CLOCK:
		case CLOCKBUTTONS://�����棺������ת
			if(k==KEY0_PRES)now=SETTIME;
			else if(k==KEY1_PRES)now=ALARMCLOCK;
			else if(k==KEY2_PRES)now=COUNT;
			else if(k==WKUP_PRES)now=SETCOUNTDOWN;
			break;
		case COUNT://�����ƣ�K0��ʼ/��ͣ K1��Ȧ K2��λ WK�˳�
			if(k==KEY0_PRES){ if(stop){stop=0;continue_=1;} else stop=1; }
			else if(k==KEY1_PRES)lap_flag=1;
			else if(k==KEY2_PRES){restart=1;stop=0;}
			else if(k==WKUP_PRES)now=CLOCK;
			break;
		case COUNTDOWN://����ʱ��K0��ʼ/��ͣ WK�˳�
			if(k==KEY0_PRES){ if(stop){stop=0;continue_=1;} else stop=1; }
			else if(k==WKUP_PRES)now=CLOCK;
			break;
		default://�����ӽ��棺WK_UP����������
			if(k==WKUP_PRES){confirm=0;now=CLOCK;}
			break;
	}
}
void run_menu(void){
	   now=CLOCK;
    while(1){
		 switch(now){
			 case CLOCK:show_clock();break;
			 case SETTIME:set_time_menu();break;
			 case SETTIMECLICK:set_time_click();break;
			 case SETTIMEUSART:set_time_usart();break;
			 case ALARMCLOCK:set_alarm_clock_menu();break;
			 case ADDALARMCLOCK:add_alarm_clock_gui();break;
			 case DROPALARMCLOCK:drop_alarm_clock_gui();break;
			 case COUNT:count_menu();break;
			 case COUNTDOWN:count_down();break;
			 case SETCOUNTDOWN:set_count_down_menu();break;
			 case CLOCKBUTTONS:clock_with_buttons();break;
		 }
		 delay_ms(100);
		 } /* */
}
