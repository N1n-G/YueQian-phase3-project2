#ifndef __INCLUDES_H__
#define __INCLUDES_H__

/* 标准C库*/
#include <stdio.h>	
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "tft.h"
#include "pic.h"
#include "key.h"
#include "adc.h"
#include "beep.h"
#include "rtc.h"
#include "dht11.h"
#include "touch.h" 
#include "max30102.h"
#include "i2c.h"
#include "algorithm.h"
#include "blue_tooth.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"


extern GPIO_InitTypeDef  	GPIO_InitStructure;
extern NVIC_InitTypeDef 	NVIC_InitStructure;		
extern SPI_InitTypeDef  	SPI_InitStructure;
extern USART_InitTypeDef 	USART_InitStructure;
extern EXTI_InitTypeDef  	EXTI_InitStructure;


typedef enum __key_fn_t
{
	KEY_NONE=0,	
	KEY_UP,
	KEY_DOWN,
	KEY_ENTER,
	KEY_BACK,
}key_fn_t;

typedef struct __task_t
{
	TaskFunction_t pxTaskCode;
	const char * const pcName;		
	const configSTACK_DEPTH_TYPE usStackDepth;
	void * const pvParameters;
	UBaseType_t uxPriority;
	TaskHandle_t * const pxCreatedTask;
}task_t;


#include "menu.h"

typedef struct __beep_t
{
	uint32_t sta;				//1-工作 0-停止
	uint32_t duration;			//持续时间，单位毫秒
}beep_t;

/* 宏定义 */
#define EVENT_GROUP_KEY1_DOWN		0x01
#define EVENT_GROUP_KEY2_DOWN		0x02
#define EVENT_GROUP_KEY3_DOWN		0x04
#define EVENT_GROUP_KEY4_DOWN		0x08
#define EVENT_GROUP_KEYALL_DOWN		0x0F

#define EVENT_GROUP_RTC_WAKEUP		0x10
#define EVENT_GROUP_RTC_ALARM		0x20

#define EVENT_GROUP_TP				0x40

#define EVENT_GROUP_FN_TP_HIT				0x80
#define EVENT_GROUP_FN_TP_SLIDE_VERTICAL	0x100
#define EVENT_GROUP_FN_TP_SLIDE_LEFT		0x200
#define EVENT_GROUP_FN_TP_SLIDE_RIGHT		0x400

#define EVENT_GROUP_FN_KEY_UP		0x10000
#define EVENT_GROUP_FN_KEY_DOWN		0x20000
#define EVENT_GROUP_FN_KEY_ENTER	0x40000
#define EVENT_GROUP_FN_KEY_BACK		0x80000



#define QUEUE_BEEP_LEN    			4   	/* 队列的长度，最大可包含多少个消息 */

#define DEBUG_PRINTF_EN 			1

#define FLAG_DHT_GET_NONE			0
#define FLAG_DHT_GET_TEMP			1
#define FLAG_DHT_GET_HUMI			2

#define FLAG_RTC_GET_NONE			0
#define FLAG_RTC_GET_DATE			1
#define FLAG_RTC_GET_TIME			2

#define FLAG_BLE_STATUS_NONE        0 //蓝牙未连接
#define FLAG_BLE_STATUS_CONNECT     1 //蓝牙已连接

#define FLAG_MPU6050_GET_NONE		0
#define FLAG_MPU6050_GET_STEP		1
#define FLAG_MPU6050_GET_STOP		2

#define TP_SLIDE_NONE				0
#define TP_SLIDE_UP					1
#define TP_SLIDE_DOWN				2
#define TP_SLIDE_LEFT				3
#define TP_SLIDE_RIGHT				4



#define MAX_BRIGHTNESS 255

/* 互斥型信号量句柄 */
extern SemaphoreHandle_t g_mutex_printf;
extern SemaphoreHandle_t g_mutex_lcd;

/* 事件标志组句柄 */
extern EventGroupHandle_t g_event_group;	

/* 消息队列句柄 */
extern QueueHandle_t 	g_queue_led;
extern QueueHandle_t 	g_queue_beep;

/* 任务句柄 */ 
extern TaskHandle_t app_task_init_handle;
extern TaskHandle_t app_task_key_handle ;
extern TaskHandle_t app_task_menu_handle ;
extern TaskHandle_t app_task_adc_handle ;
extern TaskHandle_t app_task_beep_handle ;
extern TaskHandle_t app_task_rtc_handle ;
extern TaskHandle_t app_task_dht_handle;
extern TaskHandle_t app_task_max30102_handle;
extern TaskHandle_t app_task_ble_handle;
extern TaskHandle_t app_task_mpu6050_handle;

/* 变量 */
extern volatile uint32_t g_dht_get_what;
extern volatile uint32_t g_rtc_get_what;

extern volatile uint32_t g_ble_status;//蓝牙状态
extern volatile uint32_t g_mpu6050_get_what;

extern volatile uint8_t nw;

/* 函数 */
extern void dgb_printf_safe(const char *format, ...);
extern void menu_show_main_1(void);
extern void menu_show_main_2(void);

/* OLED互斥锁高度封装 */
#define LCD_SAFE(__CODE)                                \
		do                                               \
		{                                                \
			xSemaphoreTake(g_mutex_lcd, portMAX_DELAY); \
			__CODE;                                  	\
			xSemaphoreGive(g_mutex_lcd);                \
		} while (0)                                      \


#endif
