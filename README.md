# 多功能电子钟设计

基于 **STM32F407ZGT6** 的多功能电子钟嵌入式项目，使用 Keil MDK + 正点原子（ALIENTEK）标准外设库开发，配备 TFT LCD 电容触摸屏图形界面。

![MCU](https://img.shields.io/badge/MCU-STM32F407ZGT6-03234B)
![IDE](https://img.shields.io/badge/IDE-Keil%20MDK%20uVision5-1B4F72)
![Lib](https://img.shields.io/badge/Library-STD%20Periph%20Library-2D6A4F)
![Display](https://img.shields.io/badge/Display-TFT%20LCD%20%2B%20Touch-C77DFF)

---

## 功能特性

| 功能 | 说明 |
|------|------|
| 🕐 指针式时钟 | 圆形表盘 + 时/分/秒三针，100ms 刷新，含 12 小时刻度与 60 分钟细刻度 |
| 📅 日期时间显示 | `Date:YYYY-MM-DD` / `week:X` / `Time:HH:MM:SS` |
| 🌡️ 温度显示 | STM32F407 内部温度传感器（ADC1 通道 5），多次采样取平均，支持负温 |
| ⚙️ 时间设置 | 触摸屏 +/- 按钮调整，或串口 `HH:MM` 输入 |
| 🔔 闹钟系统 | 最多 6 个闹钟，支持星期/时/分，RTC Alarm A 硬件中断触发，蜂鸣器 + LED 提示 |
| ⏱️ 正计时 / 秒表 | stop / continue / restart / exit，支持 5 组计次记录 |
| ⏳ 倒计时 | 时/分/秒可设，归零自动返回主界面 |
| 🕛 整点报时 | 每小时 0 分且秒数 < 5 时蜂鸣器 + LED 提示（闹钟激活时自动抑制） |
| 👆 触摸交互 | 兼容 GT9147 / FT5206 / OTT2001A 三种电容触摸 IC，最多 5 点触控 |
| 💻 串口调试 | USMART 组件，支持远程函数调用、参数解析与执行耗时统计 |

---

## 硬件平台

| 项目 | 参数 |
|------|------|
| 主控 | STM32F407ZGT6，Cortex-M4，168MHz，1MB Flash / 192KB RAM |
| 显示屏 | TFT LCD，FSMC 8080 并口驱动 |
| 触摸 | 软件 IIC，电容触摸（GT9147 / FT5206 / OTT2001A） |
| 时钟源 | LSE 外部 32.768kHz 晶振 → RTC |
| EEPROM | AT24C02（IIC），用于闹钟数据掉电保存 |
| 外设 | LED0(PF9) / LED1(PF10)、BEEP(PF8)、KEY0~KEY2(PE4/PE3/PE2)、WK_UP(PA0) |
| 调试 | J-Link / ST-Link |

---

## 目录结构

```
多功能电子钟设计/
├── CORE/          # ARM Cortex-M4 内核文件与启动汇编
├── FWLIB/         # STM32F4 标准外设库（inc + src）
├── SYSTEM/        # delay / sys / usart 基础模块
├── HARDWARE/      # 外设驱动：LED、LCD、BEEP、KEY、RTC、ADC、TOUCH、IIC、24CXX、EXTI、TPAD
├── GUI/           # 手写 GUI 控件库：button、edit、listbox、progressbar、scrollbar、memo、window
├── USER/          # 应用层：main.c、gui.c（核心业务）、RTC.uvprojx（Keil 工程）
├── USMART/        # 串口命令行调试组件
├── OBJ/           # 编译输出（仅保留可烧录的 RTC.hex）
├── keilkilll.bat  # 清理编译中间文件脚本（Windows）
└── 项目结构文档.md # 详细设计文档
```

---

## 状态机设计

全局变量 `now` 驱动 11 个界面状态：

| 状态 | 值 | 界面函数 |
|------|----|----------|
| `CLOCK` | 1 | `show_clock()` — 时钟主界面 |
| `SETTIME` | 2 | `set_time_menu()` — 设置方式选择 |
| `SETTIMECLICK` | 3 | `set_time_click()` — 触摸调整时间 |
| `SETTIMEUSART` | 4 | `set_time_usart()` — 串口设置时间 |
| `ALARMCLOCK` | 5 | `set_alarm_clock_menu()` — 闹钟管理 |
| `ADDALARMCLOCK` | 6 | `add_alarm_clock_gui()` — 添加闹钟 |
| `DROPALARMCLOCK` | 7 | `drop_alarm_clock_gui()` — 删除闹钟 |
| `COUNTDOWN` | 8 | `count_down()` — 倒计时运行 |
| `COUNT` | 9 | `count_menu()` — 正计时秒表 |
| `SETCOUNTDOWN` | 10 | `set_count_down_menu()` — 设置倒计时 |
| `CLOCKBUTTONS` | 11 | `clock_with_buttons()` — 带按钮主界面 |

主循环 `run_menu()` 以 100ms 为周期轮询，根据 `now` 分派界面函数，未使用 RTOS。

---

## 编译与烧录

1. 使用 **Keil uVision5** 打开 [`USER/RTC.uvprojx`](USER/RTC.uvprojx)
2. 目标器件：`STM32F407ZG`，主频 168MHz
3. 编译（F7）生成 `OBJ/RTC.hex`（烧录文件）与 `OBJ/RTC.axf`（调试文件）
4. 使用 J-Link / ST-Link 下载至开发板
5. 若只需快速验证，可直接烧录仓库内已编译好的 [`OBJ/RTC.hex`](OBJ/RTC.hex)
6. Windows 下可运行 `keilkilll.bat` 清理编译中间文件

> ⚠️ 注意：Keil 工程内的绝对路径依赖本地环境，若从其他机器克隆，请在
> `Options for Target → C/C++ → Include Paths` 中重新确认各目录包含路径。

---

## 关键实现说明

- **时钟绘制**：`LCD_Draw_Circle` 画表盘（半径 200，中心 (250,250)），指针用带宽度参数的画线函数实现，并绘制白色描边提升对比度。
- **时间合法性**：`get_max_day()` 结合闰年规则（4 整除且非 100 整除，或 400 整除）自动校验每月天数，星期由年月日推算。
- **闹钟调度**：`RTC_Set_AlarmA` 配置硬件闹钟中断；当天闹钟已过时 `get_next_alarm_clock()` 自动顺延至下周；闹钟列表通过 AT24C02 持久化。
- **触摸适配**：`_m_tp_dev` 结构体内含 `init` / `scan` / `adjust` 函数指针，运行时按读到的触摸 IC ID 绑定对应驱动。
- **画面刷新**：未使用 RTOS，采用「整屏重绘 + 局部刷新」结合的方式，避免闪烁。

更详细的设计说明（数据结构、依赖关系图、全局变量一览）请参阅 [`项目结构文档.md`](项目结构文档.md)。

---

## 开发说明

本项目基于正点原子（ALIENTEK）探索者 STM32F407 开发板的例程框架二次开发，
`CORE/`、`FWLIB/`、`SYSTEM/`、`HARDWARE/` 中的基础驱动源自官方标准库例程，
`USER/gui.c`、`USER/gui.h` 及界面逻辑为原创实现。

## License

仅供学习与交流使用。
