#include "lcd.h"
#include "math.h"
#include "stdio.h"
#include "rtc.h"
#include "adc.h"
#include "beep.h"
#include "delay.h"
#include "led.h"
#include "string.h"
#include "tpad.h"
#include "touch.h"
#include "key.h"

// ===== Unified UI Theme (light & minimal) =====
// All values are RGB565 (see lcd.h color macros).
#define THEME_BG        0xEF7D   // global background, soft light gray
#define THEME_PANEL     0xFFFF   // card / panel background, white
#define THEME_BTN       0x3D9F   // normal button, soft blue
#define THEME_BTN_HI    0x6E7F   // button top highlight band
#define THEME_BTN_SEL   0x1CDF   // selected / active button
#define THEME_ACCENT    0x2DCC   // accent button (confirm / start), teal-green
#define THEME_DANGER    0xC9C7   // cancel / danger button, soft red
#define THEME_EDGE      0xCE59   // border / divider line, light gray
#define THEME_SHADOW    0xBDD7   // bottom/right shadow line under buttons
#define THEME_TEXT      0x2104   // primary text on light bg, dark gray
#define THEME_TEXT_LT   0xFFFF   // text on dark buttons, white
#define THEME_SUB       0x7BEF   // secondary text / dial ticks, mid gray
#define THEME_HOUR      0x3D9F   // clock hour hand
#define THEME_MIN       0x2DCC   // clock minute hand
#define THEME_SEC       0xF9A6   // clock second hand, soft red
#define THEME_DIAL_RING 0xA514   // outer ring / stronger dial border
#define THEME_DIAL_IN   0xF77D   // inner dial fill, warm light tone
#define THEME_INFO_BAR  0xF75B   // info strip background
#define THEME_INFO_TAG  0x3D9F   // left tag block color

// ----- dial face refinement -----
#define THEME_DIAL_BEZEL  0x4B0D   // dark outer bezel ring, deep blue-gray
#define THEME_DIAL_RING2  0xC638   // soft mid ring between bezel and face
#define THEME_DIAL_FACE   0xFFFF   // dial face fill, clean white
#define THEME_DIAL_INSET  0xE73C   // faint inner ring drawn on the face
#define THEME_TICK_MAJOR  0x2986   // 12 hour ticks, strong dark slate
#define THEME_TICK_MINOR  0xB596   // 60 minute ticks, light gray
#define THEME_DIAL_NUM    0x18C3   // hour numbers, near-black ink

// ----- hands refinement -----
#define THEME_HAND_EDGE   0xFFFF   // white outline under each hand for contrast
#define THEME_HAND_TAIL   0xC618   // second-hand counterweight tail
#define THEME_CAP_OUT     0x2986   // hub cap outer ring, dark slate
#define THEME_CAP_IN      0xFD20   // hub cap center dot, warm amber

// ----- info area (date / time / temperature) -----
#define THEME_INFO_DATE   0x2DCC   // date tag block, teal-green
#define THEME_INFO_TIME   0x3D9F   // time tag block, soft blue
#define THEME_INFO_TEMP   0xFC60   // temperature tag block, warm orange
#define THEME_INFO_LABEL  0xFFFF   // tag label text, white
#define THEME_INFO_VALUE  0x2104   // value text on white panel, dark gray
#define THEME_INFO_DIV    0xE71C   // thin divider between info rows

#define CLOCK 1
#define SETTIME 2
#define SETTIMECLICK 3
#define SETTIMEUSART 4
#define ALARMCLOCK 5
#define ADDALARMCLOCK 6
#define DROPALARMCLOCK 7
#define COUNTDOWN 8
#define COUNT 9
#define SETCOUNTDOWN 10 
#define CLOCKBUTTONS 11
struct button{
int x;
int y;
int x1;
int y1;
int color;
char str[20];
};
void draw_point(int x1, int y1, u16 color);
void fill_circle(int uRow,int uCol,int size,u16 color);
void draw_bline(u16 x1, u16 y1, u16 x2, u16 y2, u8 size, u16 color);
void back(void);
void clock(void);
void show_date(void);
void draw_info_frame(void);
void show_time(void);
void hour_chime(void);
void draw_time(void);
void draw_hour(void);
void draw_minute(void);
void draw_second(void);
void clear_time_area(void);
void show_temprature(void);
void set_time_menu(void);
void show_clock(void);
void run_menu(void);
void set_time_click(void);
void add_alarm_clock_gui(void);
void drop_alarm_clock_gui(void);
void draw_button(struct button b);
void draw_panel(int x,int y,int x1,int y1);
void key_handle(void);
void clock_buttons(void);
void clock_buttons_click(void);
void count_menu(void);
void count_down(void);
void set_count_down_menu(void);
void record_lap(int h,int m,int s);
void show_one_lap(int idx);
void clear_laps(void);
