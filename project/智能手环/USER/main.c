/****************************************************************
*名    称:“心动轨迹”智能健康手环 （FreeRTOS综合代码实例-LCD触屏版本）(1.69英寸，触屏芯片cst816t)
*作    者:莫宁
*创建日期:2024/10/30
*说  明:	
	1.当前代码规范、结构清晰，尽可能实现了模块化编程，一个任务管理一个硬件。
	  该代码框架沿用《FreeRTOS综合代码实例-OLED版本》，重点将OLED显示修改为LCD显示
	2.菜单项对应硬件模块功能
		菜单第1页
			.时间显示 		[已实现]
			.温湿度显示  	[已实现]
			.设置（灯光、电量显示、设置时间）	[实现了设置时间]	
			
		菜单第2页
			.步数计算		[已实现]
			.久坐提醒		[已实现]
			.心率血氧显示  	[已实现]
			
		菜单第3页
			.GAME			[有空再实现]
*操作说明:
	1.按键功能
		.按键1与按键2：菜单项的选择
		.按键3：进入菜单项
		.按键4：退出菜单项
	2.可调电阻
		.旋钮式选择对应的菜单，类似iPod
	3.蜂鸣器
		.执行对应的操作会嘀一声示意
	4.菜单项实现
		.多级菜单的访问
	5.自动熄屏
		.若无操作，过一会自动熄屏
		.按下任意按键唤醒屏幕
	6.触屏
		.单击：进入菜单项
		.上滑/下滑：菜单项上下选择
		.右滑：切换到后一个同级菜单
		.左滑：切换到前一个同级菜单
		
*注意事项:	

	1.LCD
	（1）当前代码只能使用模拟SPI，原因该硬件使用量9bit SPI传输格式(D/C xxxx xxxx，bit8为数据/命令 bit7~bit0为内容)
	     但STM32的硬件SPI只能支持8bit/16bit的传输格式
	2.触摸屏
	（1）由于该屏幕较小，触屏时可能存在误触
	（2）该触摸芯片较特殊，一直按压屏幕，会不断触发中断，不会一直低电平，跟电阻屏触屏芯片有区别
	（3）检查触点是否离开屏幕，需要读取该触点的坐标值高字节[7:6]去判断
		
*关键函数：
	1.app_task_menu，文件路径：main.c
		.负责对菜单项选择/进入/退出的管理
		.显示光标指示当前菜单项
		
	2.menu_show,文件路径：menu.c
		.显示菜单项图标
		.显示菜单项文字
		
*关键变量类型：	
	1.menu_t，文件路径：menu.h
		.链式存储水平左/右菜单、父/子菜单
		
*关键宏：	
	1.DEBUG_PRINTF_EN，文件路径：includes.h
		.使能则通过串口1输出调试信息
		
	2.LCD_SAFE，文件路径：includes.h
		.使用互斥锁保护LCD访问
*****************************************************************/	
#include "includes.h"

TaskHandle_t app_task_init_handle = NULL;
TaskHandle_t app_task_key_handle = NULL;
TaskHandle_t app_task_menu_handle = NULL;
TaskHandle_t app_task_adc_handle = NULL;
TaskHandle_t app_task_beep_handle = NULL;
TaskHandle_t app_task_tp_handle = NULL;
TaskHandle_t app_task_rtc_handle = NULL;
TaskHandle_t app_task_dht_handle = NULL;
TaskHandle_t app_task_ble_handle = NULL;
TaskHandle_t app_task_max30102_handle = NULL;
TaskHandle_t app_task_mpu6050_handle = NULL;


/* 软件定时器句柄 */
TimerHandle_t soft_timer_Handle = NULL;

GPIO_InitTypeDef  	GPIO_InitStructure;
USART_InitTypeDef 	USART_InitStructure;
NVIC_InitTypeDef 	NVIC_InitStructure;
SPI_InitTypeDef  	SPI_InitStructure;
EXTI_InitTypeDef  	EXTI_InitStructure;
RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;


volatile uint32_t g_dht_get_what = FLAG_DHT_GET_NONE;
volatile uint32_t g_rtc_get_what = FLAG_RTC_GET_NONE;
volatile uint32_t g_ble_status = FLAG_BLE_STATUS_NONE;//蓝牙连接状态
volatile uint32_t g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
volatile uint32_t g_system_no_opreation_cnt = 0;
volatile uint32_t g_system_display_on = 1;
volatile uint32_t ble_connect_get = 0;


//max30102任务的变量
volatile uint32_t aun_ir_buffer[500]; //IR LED sensor data
volatile int32_t n_ir_buffer_length;    //data length
volatile uint32_t aun_red_buffer[500];    //Red LED sensor data
volatile int32_t n_sp02; //SPO2 value
volatile int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
volatile int32_t n_heart_rate;   //heart rate value
volatile int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
volatile uint8_t uch_dummy;

tp_t g_tp;

/* 消息队列句柄 */
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_usart;

// 添加任务句柄
TaskHandle_t app_task_ble_status_handle = NULL;

/* 事件标志组句柄 */
EventGroupHandle_t g_event_group;

/* 互斥型信号量句柄 */
SemaphoreHandle_t g_mutex_printf;
SemaphoreHandle_t g_mutex_lcd;

/* 任务*/ 
static void app_task_init(void* pvParameters); 
static void app_task_tp(void* pvParameters); 
static void app_task_key(void* pvParameters); 
static void app_task_menu(void* pvParameters); 
static void app_task_adc(void* pvParameters); 
static void app_task_beep(void* pvParameters); 
static void app_task_rtc(void* pvParameters); 
static void app_task_dht(void* pvParameters); 
static void app_task_ble(void* pvParameters); 
static void app_task_ble_status(void *pvParameters);
static void app_task_max30102(void* pvParameters); 
static void app_task_mpu6050(void* pvParameters);


/* 软件定时器 */
static void soft_timer_callback(TimerHandle_t pxTimer);

void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN

	va_list args;
	va_start(args, format);

	/* 获取互斥信号量 */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* 释放互斥信号量 */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

/* 任务列表 */
static const task_t task_tbl[] = {
	{app_task_tp, 				"app_task_tp", 				512, NULL, 7, &app_task_tp_handle},	
	{app_task_key, 				"app_task_key", 			512, NULL, 5, &app_task_key_handle},
	{app_task_menu, 			"app_task_menu", 			512, &menu_main_tbl, 10, &app_task_menu_handle},
	{app_task_adc, 				"app_task_adc", 			512, NULL, 5, &app_task_adc_handle},
	{app_task_beep, 			"app_task_beep", 			512, NULL, 5, &app_task_beep_handle},
	{app_task_rtc, 				"app_task_rtc", 			512, NULL, 5, &app_task_rtc_handle},
	{app_task_dht, 				"app_task_dht", 			512, NULL, 5, &app_task_dht_handle},
	{app_task_ble,				"app_task_ble",				512, NULL, 5, &app_task_ble_handle},
	{app_task_max30102, 		"app_task_max30102", 		512, NULL, 5, &app_task_max30102_handle},
	{app_task_mpu6050, 			"app_task_mpu6050", 		1024, NULL, 5, &app_task_mpu6050_handle},
	{0, 0, 0, 0, 0, 0}
};


int main(void)
{
	/* 设置系统中断优先级分组4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* 系统定时器中断频率为configTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);									
	
	/* 初始化串口1 */
	usart_init(115200);     							

	/* 创建app_task_init任务 */
	xTaskCreate((TaskFunction_t )app_task_init,  	/* 任务入口函数 */
			  (const char*    )"app_task_init",		/* 任务名字 */
			  (uint16_t       )512,  				/* 任务栈大小 */
			  (void*          )NULL,				/* 任务入口函数参数 */
			  (UBaseType_t    )5, 					/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_init_handle);/* 任务控制块指针 */ 

			  
	/* 开启任务调度 */
	vTaskStartScheduler(); 
	

	printf("none run here...\r\n");
			  
	while(1);

}

void lcd_startup_info(void)
{
	uint16_t x=0;
	
	LCD_SAFE(
			
		/* 清屏 */
		lcd_clear(WHITE);
		
		/* 显示主界面 */
		lcd_draw_picture((g_lcd_width-95)/2,(g_lcd_height-50)/2-15,100,100,gImage_logo_60x60);
	
		x=(g_lcd_width-4*37)/2;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,16,BLACK,WHITE,32);	// 心
		lcd_show_chn(x+20,(g_lcd_height-60)/2-50,28,BLACK,WHITE,32);// 智
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,24,BLACK,WHITE,32);		// 莫
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,25,BLACK,WHITE,32);	// 动
		lcd_show_chn(x+10,(g_lcd_height-60)/2-50,29,BLACK,WHITE,32);// 能
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,57,BLACK,WHITE,32);	// 
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,55,BLACK,WHITE,32);	// 轨
		lcd_show_chn(x,(g_lcd_height-60)/2-50,30,BLACK,WHITE,32);	// 手
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,26,BLACK,WHITE,32);		// 宁
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,56,BLACK,WHITE,32);	// 迹
		lcd_show_chn(x-10,(g_lcd_height-60)/2-50,31,BLACK,WHITE,32);// 环
	
		x=0;
		while(x<g_lcd_width)
		{
			/* 进度条 */
			lcd_fill(x,g_lcd_height-30,10,30,BLUE);

			x+=10;
			
			vTaskDelay(50);
		}
		
		
	);
}


static void app_task_init(void* pvParameters)
{

	uint32_t i=0;
	
	menu_ext_t m_ext;
	
	printf("[app_task_init] create success\r\n");
	
	/* 创建消息队列 */
	g_queue_beep  	= xQueueCreate(QUEUE_BEEP_LEN, 	sizeof(beep_t));
	
	g_queue_usart = xQueueCreate(5, 128);
	
	/* 创建互斥型信号量 */
	g_mutex_printf 	= xSemaphoreCreateMutex();		  
	g_mutex_lcd 	= xSemaphoreCreateMutex();	

	/* 创建事件标志组 */
	g_event_group = xEventGroupCreate();		
	
	/* lcd初始化 */ 
	lcd_init();
	
	/* beep初始化 */
	beep_init();

	/* 按键初始化 */
	key_init();	
	
	/* adc初始化 */
	adc_init();

	/* rtc初始化 */
	rtc_init();	
	
	/* 温湿度模块初始化 */
	dht11_init();	
	
	/* 触摸芯片初始化 */
	tp_init();	

	/* 心率血氧模块初始化 */
	max30102_init();
	
	/*蓝牙模块初始化*/
	blue_tooth_init(9600);
	
	/* MPU6050初始化 */
	MPU_Init();
	
	taskENTER_CRITICAL();
	

	/* 创建用到的任务 */
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* 任务入口函数 */
					task_tbl[i].pcName,			/* 任务名字 */
					task_tbl[i].usStackDepth,	/* 任务栈大小 */
					task_tbl[i].pvParameters,	/* 任务入口函数参数 */
					task_tbl[i].uxPriority,		/* 任务的优先级 */
					task_tbl[i].pxCreatedTask); /* 任务控制块指针 */
		
		 // 创建蓝牙状态显示任务
		xTaskCreate((TaskFunction_t)app_task_ble_status,
                (const char *)"app_task_ble_status",
                256,           // 栈大小
                NULL,          // 任务参数
                2,             // 优先级
                &app_task_ble_status_handle);
		i++;
	}
	
	taskEXIT_CRITICAL();
	
	/* 创建周期软件定时器 */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* 定时器周期 1000(tick) */
									 (UBaseType_t)pdTRUE, /* 周期模式 */
									 (void *)1,			  /* 为每个计时器分配一个索引的唯一ID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* 开启周期软件定时器 */
	xTimerStart(soft_timer_Handle, 0);	
									 
//	vTaskSuspend(app_task_ble_handle);
	
	
	/* 显示主菜单 */
	
	lcd_startup_info();	

	m_ext.menu=menu_main_1;
	
	menu_show(&m_ext);
	

	vTaskDelete(NULL);
}   

/**
 * @brief 按键任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 */
static void app_task_key(void *pvParameters)
{
	EventBits_t EventValue;

	
	
	dgb_printf_safe("[app_task_key] create success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 延时消抖 */
		vTaskDelay(20);
		
		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;		
		
		if(g_system_display_on==0)
		{
			lcd_display_on(1);
			g_system_display_on=1;
			continue;
			
		}
		
	
			

		if (EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			/* 禁止EXTI0触发中断 */
			NVIC_DisableIRQ(EXTI0_IRQn);

			/* 确认是按下 */
			if (PAin(0) == 0)
			{

				dgb_printf_safe("[app_task_key] S1 Press\r\n");

				/* 发送KEY_UP按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_DOWN);


			}
			
			while(PAin(0)==0)
					vTaskDelay(10);
			
			/* 允许EXTI0触发中断 */
			NVIC_EnableIRQ(EXTI0_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			/* 禁止EXTI2触发中断 */
			NVIC_DisableIRQ(EXTI2_IRQn);

			if (PEin(2) == 0)
			{
				dgb_printf_safe("[app_task_key] S2 Press\r\n");

				/* 发送KEY_DOWN按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_UP);
							
			}
			
			while(PEin(2)==0)
					vTaskDelay(10);			

			/* 允许EXTI2触发中断 */
			NVIC_EnableIRQ(EXTI2_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			/* 禁止EXTI3触发中断 */
			NVIC_DisableIRQ(EXTI3_IRQn);

			if (PEin(3) == 0)
			{
				dgb_printf_safe("[app_task_key] S3 Press\r\n");
			
				/* 发送KEY_ENTER按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
				
			}
			
			while(PEin(3)==0)
					vTaskDelay(10);			

			/* 允许EXTI3触发中断 */
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			/* 禁止EXTI4触发中断 */
			NVIC_DisableIRQ(EXTI4_IRQn);

			if (PEin(4) == 0)
			{

				dgb_printf_safe("[app_task_key] S4 Press\r\n");

				/* 发送KEY_BACK按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);				
				
			}
			
			while(PEin(4)==0)
					vTaskDelay(10);				

			/* 允许EXTI4触发中断 */
			NVIC_EnableIRQ(EXTI4_IRQn);
		}
	}
}

/**
 * @brief 触摸屏任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 */
static void app_task_tp(void *pvParameters)
{
	EventBits_t EventValue;	
	uint16_t screen_x=0,screen_y=0;
	uint8_t tp_sta;
	uint32_t tp_slide_event=TP_SLIDE_NONE;
	
	dgb_printf_safe("[app_task_tp] create success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_TP,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;		
		
		if(g_system_display_on==0)
		{
			lcd_display_on(1);
			g_system_display_on=1;
			continue;
		}
		
		
		
		/* 禁止TP触发中断 */
		NVIC_DisableIRQ(EXTI9_5_IRQn);
		
		if ((EventValue & EVENT_GROUP_TP))
		{
			
			while(1)
			{
				tp_sta=tp_read(&screen_x,&screen_y);
				
				g_tp.x=screen_x;			
				g_tp.y=screen_y;
				
				
				//获取点击手指数量，为0则跳出循环
				if(tp_finger_num_get() == 0)
						break;		
				
				if(tp_sta==0x01)
				{
					
					/* 设置事件标志组中的垂直滑动事件 */
					EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_VERTICAL);
					
					tp_slide_event=TP_SLIDE_DOWN;			
				}			
				
				if(tp_sta==0x02)
				{
					/* 设置事件标志组中的垂直滑动事件 */
					EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_VERTICAL);
					
					tp_slide_event=TP_SLIDE_UP;			
				}				
				
				vTaskDelay(10);
			}

		}	

		
		if(tp_sta==0x03)
		{
			/* 设置事件标志组中的向左滑动事件 */
			EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_LEFT);
			
			tp_slide_event=TP_SLIDE_LEFT;			
		}	


		if(tp_sta==0x04)
		{
			/* 设置事件标志组中的向右滑动事件 */
			EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_RIGHT);
			
			tp_slide_event=TP_SLIDE_RIGHT;		
		}

		
		if(tp_sta==0x05)
		{
			/* 点击执行事件 */
			xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_HIT);			
		}


		printf("tp_sta=%02X g_system_no_opreation_cnt=%d\r\n",tp_sta,g_system_no_opreation_cnt);
		
		/* 允许TP触发中断 */
		NVIC_EnableIRQ(EXTI9_5_IRQn);
	}
}

/**
 * @brief adc任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 通过可调电阻获取的电压值，用作转盘菜单的控制
 */
static void app_task_adc(void *pvParameters)
{
	int32_t adc_vol_last = 0;
	int32_t adc_vol = 0;
	int32_t result;

	/* 获取当前电压初值 */
	adc_vol = adc_voltage_get();
	adc_vol_last = adc_vol;

	for (;;)
	{
		/* 获取电压值 */
		adc_vol = adc_voltage_get();

		result = adc_vol - adc_vol_last;

		if (result > 200 || result < -200)
		{

			/* 发送KEY_DOWN/KEY_UP按键事件 */
			xEventGroupSetBits(g_event_group, result > 0 ? EVENT_GROUP_FN_KEY_UP : EVENT_GROUP_FN_KEY_DOWN);

			dgb_printf_safe("[app_task_adc] adc_vol_last=%d adc_vol=%d\r\n", adc_vol_last, adc_vol);

			adc_vol_last = adc_vol;
		}

		vTaskDelay(200);
	}
}

/**
 * @brief 蜂鸣器任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 接收消息队列控制蜂鸣器。
 */
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_beep] create success\r\n");

	for (;;)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* 消息队列的句柄 */
								&beep,			/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		if (xReturn != pdPASS)
			continue;

		dgb_printf_safe("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);
		
		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;				

		/* 检查蜂鸣器是否需要持续工作 */
		if (beep.duration)
		{
			BEEP(beep.sta);

			while (beep.duration--)
				vTaskDelay(10);

			/* 蜂鸣器状态翻转 */
			beep.sta ^= 1;
		}

		BEEP(beep.sta);

	}
}

static void app_task_rtc(void* pvParameters)
{
	char time_buf[16] = {0};
	char second_buf[16] = {0};
	char year_buf[16] = {0};
	char month_buf[16] = {0};
	char date_buf[16] = {0};
	EventBits_t EventValue;	
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	dgb_printf_safe("[app_task_rtc] create success and suspend self\r\n");

	vTaskSuspend(NULL);

	for(;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);		
		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;			
		
		if (EventValue & EVENT_GROUP_RTC_WAKEUP)
		{
			/* RTC_GetTime，获取时间 */
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
			/* RTC_GetDate，获取日期 */
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
			memset(time_buf, 0, sizeof time_buf);
			memset(second_buf, 0, sizeof second_buf);
			
			memset(year_buf, 0, sizeof year_buf);
			memset(month_buf, 0, sizeof month_buf);
			memset(date_buf, 0, sizeof date_buf);

			/* 格式化字符串 */
			sprintf((char *)time_buf, "%02d:%02d", RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes);
			sprintf((char *)second_buf, "%02d", RTC_TimeStructure.RTC_Seconds);

			/* 格式化字符串 */
			sprintf((char *)year_buf, "20%02d", RTC_DateStructure.RTC_Year);
			sprintf((char *)month_buf, "%02d", RTC_DateStructure.RTC_Month);
			sprintf((char *)date_buf, "%02d", RTC_DateStructure.RTC_Date);
			
			LCD_SAFE(
				lcd_show_string(70,180,time_buf,BLACK,WHITE,32,0);
				
				lcd_show_string(160,170,second_buf,BLACK,WHITE,24,0);
				
				
				lcd_show_string(30-20,240,year_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+50-20, 240,6,BLACK,WHITE,24);	// 年
				
				lcd_show_string(30+75-20,240,month_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+100-20, 240,7,BLACK,WHITE,24);	// 月
				
				lcd_show_string(30+125-20,240,date_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+150-20, 240,8,BLACK,WHITE,24);	// 日
				
				
				switch (RTC_DateStructure.RTC_WeekDay)
				{
					case 1:lcd_show_string(70,100,"MON",BLACK,WHITE,32,0);break;
					case 2:lcd_show_string(70,100,"TUE",BLACK,WHITE,32,0);break;
					case 3:lcd_show_string(70,100,"WED",BLACK,WHITE,32,0);break;
					case 4:lcd_show_string(70,100,"THU",BLACK,WHITE,32,0);break;
					case 5:lcd_show_string(70,100,"FRI",BLACK,WHITE,32,0);break;
					case 6:lcd_show_string(70,100,"SAT",BLACK,WHITE,32,0);break;
					default:lcd_show_string(70,100,"SUN",BLACK,WHITE,32,0);break;
				}
			);

			dgb_printf_safe("[app_task_time] %s:%s\r\n", time_buf, second_buf);
			dgb_printf_safe("[app_task_date] %s/%s/%s\r\n", year_buf, month_buf, date_buf);
		}
	}
}

static void app_task_dht(void* pvParameters)
{
	uint8_t dht11_data[5] = {0};
	char temp_buf[16] = {0};
	char humi_buf[16] = {0};

	int32_t rt=-1;
	uint16_t x;

	dgb_printf_safe("[app_task_dht] create success and suspend self \r\n");

	vTaskSuspend(NULL);

	dgb_printf_safe("[app_task_dht] resume success\r\n");

	for (;;)
	{
		rt=dht11_read_data(dht11_data);
		
		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;				

		if (rt==0)
		{
			memset(temp_buf, 0, sizeof temp_buf);
			memset(humi_buf, 0, sizeof humi_buf);
			
			sprintf((char *)temp_buf, "%02d.%d", dht11_data[2], dht11_data[3]);
			sprintf((char *)humi_buf, "%02d.%d%%", dht11_data[0], dht11_data[1]);
			
			/* 显示温度/湿度 */
			x=(LCD_WIDTH-strlen(temp_buf)*32/2)/2+30;

			LCD_SAFE(
				lcd_show_chn(30,160,4,BLACK,WHITE,32);
				lcd_show_chn(67,160,21,BLACK,WHITE,32);
				lcd_show_chn(180,160,36,BLACK,WHITE,32);
				
				lcd_show_chn(30,200,20,BLACK,WHITE,32);
				lcd_show_chn(67,200,21,BLACK,WHITE,32);
				
				lcd_show_string(x,160,temp_buf,BLACK,WHITE,32,0);
				lcd_show_string(x,200,humi_buf,BLACK,WHITE,32,0);
			);

			dgb_printf_safe("[app_task_dht] temp:%s\r\n", temp_buf);
			dgb_printf_safe("[app_task_dht] humi:%s\r\n", humi_buf);
		}

		vTaskDelay(6000);
	}
}

static void app_task_ble(void* pvParameters)
{
	char buf[128] = {0};
	char *p = NULL;
	uint32_t year = 0;
	uint32_t month = 0;
	uint32_t day = 0;
	uint32_t week_day = 0;
	uint32_t hours = 0;
	uint32_t minutes = 0;
	uint32_t seconds = 0;

	for (;;)
	{
		// 串口接收数据
		xQueueReceive(g_queue_usart, buf, portMAX_DELAY);

		printf("%s\r\n", buf);

		// 设置时间
		if (g_rtc_get_what == FLAG_RTC_GET_TIME || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// “TIME SET-10-20-30#”
			if (strstr((const char *)buf, "TIME"))
			{
				p = strtok((char *)buf, "-");
				// 小时10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// 分钟20
				p = strtok(NULL, "-");
				minutes = atoi(p);
				// 秒30
				p = strtok(NULL, "-");
				seconds = atoi(p);

				RTC_TimeStructure.RTC_Hours = hours;
				RTC_TimeStructure.RTC_Minutes = minutes;
				RTC_TimeStructure.RTC_Seconds = seconds;
				RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

				printf("set time ok!\r\n");

				if (g_rtc_get_what == FLAG_RTC_GET_TIME)
				{
					LCD_SAFE(
						/* 清屏 */
						lcd_clear(WHITE);
						lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
					
						lcd_draw_picture(95, 90, 48, 48, gImage_pic_success_145x145);
						lcd_show_chn(40,170,41,BLACK,WHITE,32);
						lcd_show_chn(80,170,42,BLACK,WHITE,32);
						lcd_show_chn(120,170,58,BLACK,WHITE,32);
						lcd_show_chn(160,170,43,BLACK,WHITE,32);
					);
				}
			}
			
			// “DATE SET-2023-5-25-4#”
			if (strstr((const char *)buf, "DATE"))
			{
				p = strtok((char *)buf, "-");
				p = strtok(NULL, "-"); // 2023
				year = atoi(p);
				// 提取月份
				p = strtok(NULL, "-");
				month = atoi(p);

				// 提取天数
				p = strtok(NULL, "-");
				day = atoi(p);
				// printf("p=%s",p);

				// 提取星期几
				p = strtok(NULL, "-");
				week_day = atoi(p);
				// printf("p=%s",p);

				RTC_DateStructure.RTC_Year = year - 2000;
				RTC_DateStructure.RTC_Month = month;
				RTC_DateStructure.RTC_Date = day;
				RTC_DateStructure.RTC_WeekDay = week_day;

				RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

				printf("set date ok!\r\n");

				if (g_rtc_get_what == FLAG_RTC_GET_TIME)
				{
					LCD_SAFE(
						/* 清屏 */
						lcd_clear(WHITE);
						lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
					
						lcd_draw_picture(95, 90, 48, 48, gImage_pic_success_145x145);
						lcd_show_chn(40,170,41,BLACK,WHITE,32);
						lcd_show_chn(80,170,42,BLACK,WHITE,32);
						lcd_show_chn(120,170,58,BLACK,WHITE,32);
						lcd_show_chn(160,170,43,BLACK,WHITE,32);
					);
				}
			}
			
		}
	}
}


// 定义蓝牙状态显示任务
static void app_task_ble_status(void *pvParameters)
{
    for (;;)
    {
        // 检查蓝牙连接状态
        ble_connect_get = ble_connect_sta_get();
        if (ble_connect_get == FLAG_BLE_STATUS_CONNECT)
        {
            // 显示蓝牙图标在屏幕的固定位置
            LCD_SAFE(
                lcd_draw_picture(190, 230, 48, 48, gImage_ble_48x48);
            );
        }
		else
		{
			if(nw == 1)
			{
				LCD_SAFE(
					lcd_fill(190, 230, 50, 50,BLACK);
				);
			}
			else if(nw == 2)
			{
				LCD_SAFE(
					lcd_draw_picture(190, 230, 50, 50, gImage_pic_remain_w_50x50);
				);
			}
			else
			{
				LCD_SAFE(
					lcd_fill(190, 230, 50, 50,WHITE);
				);
			}
			
		}
		

        // 延时一段时间，避免频繁刷新（比如500毫秒）
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void app_task_max30102(void* pvParameters)
{
	char buff[32] = {0};
	char bufff[32] = {0};
	uint32_t un_min, un_max, un_prev_data;  
	int32_t i;
	int32_t n_brightness;
	float f_temp;
	
	uint8_t temp[6];
	
	un_min=0x3FFFF;
	un_max=0;
	
	n_ir_buffer_length=500; //buffer length of 100 stores 5 seconds of samples running at 100sps
	//read the first 500 samples, and determine the signal range
	
	//vTaskSuspend(NULL);
    for(i=0;i<n_ir_buffer_length;i++)
    {
        while(MAX30102_INT==1);   //wait until the interrupt pin asserts
        
		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
            
        if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];    //update signal min
        if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];    //update signal max
    }
	un_prev_data=aun_red_buffer[i];
	//calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
	
	//BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_max30102] create success and suspend self\r\n");

	vTaskSuspend(NULL);

	dgb_printf_safe("[app_task_max30102] stay\r\n");
	
	for(;;)
	{
		i=0;
        un_min=0x3FFFF;
        un_max=0;
		n_ir_buffer_length=500;
		
		/* dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
		
		   将前100组样本转储到存储器中，并将最后400组样本移到顶部
		*/
		
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            /* update the signal min and max 
			   更新信号最小值和最大值
			*/
			
            if(un_min>aun_red_buffer[i])
				un_min=aun_red_buffer[i];
			
            if(un_max<aun_red_buffer[i])
				un_max=aun_red_buffer[i];
        }
		
		/* take 100 sets of samples before calculating the heart rate 
		
		   在计算心率之前采集100组样本
		*/
		
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
			
            while(MAX30102_INT==1);
			
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			
			/* 组合值以获得实际数字 */
			aun_red_buffer[i] =  ((temp[0]&0x03)<<16) |(temp[1]<<8) | temp[2];   
			aun_ir_buffer[i] =   ((temp[3]&0x03)<<16) |(temp[4]<<8) | temp[5];   
        
            if(aun_red_buffer[i]>un_prev_data)
            {
                f_temp=aun_red_buffer[i]-un_prev_data;
				
                f_temp/=(un_max-un_min);
				
                f_temp*=MAX_BRIGHTNESS;
				
                n_brightness-=(int32_t)f_temp;
				
                if(n_brightness<0)
                    n_brightness=0;
            }
            else
            {
                f_temp=un_prev_data-aun_red_buffer[i];
				
                f_temp/=(un_max-un_min);
				
                f_temp*=MAX_BRIGHTNESS;
				
                n_brightness+=(int32_t)f_temp;
				
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }
		}
		
		/* 计算心率和血氧饱和度 */
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		
		/* 通过UART将样本和计算结果发送到终端程序 */
		if((ch_hr_valid == 1)&& (n_heart_rate>=60) && (n_heart_rate<100))
		{

			dgb_printf_safe("心率=%d\r\n", n_heart_rate);
			
			sprintf(buff, ":%d", n_heart_rate);
			
			LCD_SAFE(
				lcd_show_chn(60,160,16,BLACK,WHITE,32);
				lcd_show_chn(92,160,17,BLACK,WHITE,32);
				
				/* 显示心率 */
				lcd_show_string(130,160,buff,BLACK,WHITE,32,0);
			);
			
			memset(buff, 0, sizeof buff);
		}

		
		if((ch_spo2_valid ==1)&& (n_sp02>=95) && (n_sp02<100))
		{
			dgb_printf_safe("血氧浓度=%d\r\n", n_sp02);

			sprintf(bufff, ":%d", n_sp02);
			
			LCD_SAFE(
				lcd_show_chn(30,200,18,BLACK,WHITE,32);
				lcd_show_chn(62,200,19,BLACK,WHITE,32);
				
				lcd_show_chn(94,200,35,BLACK,WHITE,32);
				lcd_show_chn(126,200,21,BLACK,WHITE,32);
				
				/* 显示血氧浓度 */
				lcd_show_string(160,200,bufff,BLACK,WHITE,32,0);
			);
			
			memset(bufff, 0, sizeof bufff);	
		}			
		
		delay_ms(100);	
	}
}


static void app_task_mpu6050(void* pvParameters)
{
	char buf[16] = {0};
	unsigned long  step_count_last=0;
	unsigned long  step_count=0;
	uint32_t sedentary_event=0;
	uint32_t t=0;
	float pitch, roll, yaw; /* 欧拉角 */

	dgb_printf_safe("[app_task_mpu6050] create success and suspend self \r\n");

	//vTaskSuspend(NULL);

	dgb_printf_safe("mpu6050持续工作中......r\n");

	while(mpu_dmp_init())
	{
		delay_ms(100);
	}
	
	/* 设置步数初值为0*/
	while(dmp_set_pedometer_step_count(0))
	{
		delay_ms(100);
	}

	for (;;)
	{
		/* 获取步数 */
		dmp_get_pedometer_step_count(&step_count);
		
		//dgb_printf_safe("mpu6050== %dr\n",step_count);
		//dgb_printf_safe("[INFO] 当前步数:%ld 以前步数:%ld\r\n",step_count,step_count_last);
		
		step_count_last=step_count;
		
		sprintf((char *)buf, ":%d",step_count);
		// dgb_printf_safe("%d\r\n",g_mpu6050_get_what);
		if (g_mpu6050_get_what == FLAG_MPU6050_GET_STEP)
		{
			/* 当前步数 */
			lcd_show_chn(30,180,44,BLACK,WHITE,32);
			lcd_show_chn(62,180,45,BLACK,WHITE,32);
			
			lcd_show_chn(94,180,46,BLACK,WHITE,32);
			lcd_show_chn(126,180,47,BLACK,WHITE,32);
				
			lcd_show_string(160,190,buf,BLACK,WHITE,32,0);
			
			delay_ms(200);			
			
		}
				

		if(g_mpu6050_get_what == FLAG_MPU6050_GET_STOP)
		{
			t++;
		
			/* 若指定秒数内没有行走，触发久坐事件 */
			if(t>=100)
			{
				/* 检查步数的变化 */
				if((step_count - step_count_last) < 5)
				{
					/* 步数变化不大，则设置久坐事件标志位为1 */
					sedentary_event=1;
				}
				
				dgb_printf_safe("[INFO] 当前步数:%ld 以前步数:%ld\r\n",step_count,step_count_last);
				
				step_count_last=step_count;
				
				t=0;
			}
			
			if(sedentary_event)
			{
				sedentary_event=0;
				
				dgb_printf_safe("[INFO] 坐立过久，请站起来走走...\r\n");
				
				
				lcd_draw_picture(30,30,182,200,gImage_sedentary_remind_170x135);
				
				nw = 0;

				PFout(8)=1;
				PFout(9)=0;
				delay_ms(100);
				PFout(8)=0;
				PFout(9)=1;	
			}
		
		}
		memset(buf, 0, sizeof buf);
		
		/* 抬手亮屏 */
		if(0 == mpu_dmp_get_data(&pitch, &roll, &yaw))
		{
			/* 延时一会 */
			delay_ms(200);
			
			if(roll > 15 && !g_system_display_on)
			{
				/* 进入临界区 */
				taskENTER_CRITICAL();
				
				g_system_display_on=1;

				/* 清零统计系统无操作计数值 */
				g_system_no_opreation_cnt=0;

				/* 退出临界区 */
				taskEXIT_CRITICAL();

				dgb_printf_safe("抬手亮屏\r\n");

				/* 点亮OLED屏 */
				lcd_display_on(1);				
			}
		}
				
	}
}


static void app_task_menu(void* pvParameters)
{
	EventBits_t EventValue;
	
	tp_t tp={0};
	
	beep_t beep;	
	
	uint32_t item_total;			//记录当前菜单有多少个项目
	uint32_t item_cursor_last = 0;	//记录当前菜单此前指向哪个项目
	uint32_t item_cursor = 0;		//记录当前菜单指向哪个项目
	uint32_t fun_run = 0;			//记录是否有项目功能函数在执行

	menu_ext_t m_ext;

	menu_t *m;

	/* 通过参数传递获取主菜单页指针 */
	menu_t **m_main = (menu_t **)pvParameters;		
	

	
	/* 配置m_ext指向的当前菜单及相关参数 */
	m_ext.menu = *m_main;						
	m_ext.key_fn = KEY_NONE;
	m_ext.item_cursor = item_cursor;
	m_ext.item_total = menu_item_total(m_ext.menu);	
	
	
	/* 配置记录菜单指针m */
	m = m_ext.menu;	
	
	dgb_printf_safe("[app_task_menu] create success\r\n");
	
	for(;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)    EVENT_GROUP_FN_KEY_UP \
										 				| EVENT_GROUP_FN_KEY_DOWN \
										 				| EVENT_GROUP_FN_KEY_ENTER \
										 				| EVENT_GROUP_FN_KEY_BACK\
														| EVENT_GROUP_FN_TP_HIT\
														| EVENT_GROUP_FN_TP_SLIDE_VERTICAL\
														| EVENT_GROUP_FN_TP_SLIDE_LEFT\
														| EVENT_GROUP_FN_TP_SLIDE_RIGHT,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		
		/* 清零统计系统无操作计数值 */
		g_system_no_opreation_cnt = 0;

		/* 触屏-左滑事件 */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_LEFT)	
		{
			
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_LEFT \r\n");
			
			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				/* 发送KEY_BACK按键事件 */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);		

				continue;
			}	
			
			/* 若main菜单有同级的左侧菜单 */
			if (m->same_left)
			{
				dgb_printf_safe("[app_task_menu] menu main switch to the left menu\r\n");
				
				item_cursor = 0;

				m_main--;

				m_ext.menu = *m_main;
				m_ext.key_fn = KEY_NONE;
				m_ext.item_cursor = 0;
				m_ext.item_total = menu_item_total(m_ext.menu);

				m = m_ext.menu;

				/* 显示菜单内容 */
				menu_show(&m_ext);
				
				/* 嘀一声示意 */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
							&beep,		   /* 发送的消息内容 */
							100);		   /* 等待时间 100 Tick */				
				
										
			}
			
			/* 父菜单有效 */
			if (m->parent)
			{
				/* 指向父菜单 */
				m = m->parent;

				/* 保存当前菜单 */
				m_ext.menu = m;

				/* 复位光标值 */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : NULL);

				fun_run = 0;

				/* 显示当前菜单 */
				menu_show(&m_ext);
				
				/* 嘀一声示意 */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
							&beep,		   /* 发送的消息内容 */
							100);		   /* 等待时间 100 Tick */				
				
							
			}			
		}
		
		/* 触屏-右滑事件 */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_RIGHT)	
		{
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_RIGHT \r\n");
			
			/* 若main菜单有同级的右侧菜单 */
			if (m->same_right)
			{
				dgb_printf_safe("[app_task_menu] menu main switch to the right menu\r\n");
				item_cursor = 0;
				m_main++;

				m_ext.menu = *m_main;
				m_ext.key_fn = KEY_NONE;
				m_ext.item_cursor = 0;
				m_ext.item_total = menu_item_total(m_ext.menu);

				m = m_ext.menu;
				

				/* 显示菜单内容 */
				menu_show(&m_ext);
				
				
				/* 嘀一声示意 */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
							&beep,		   /* 发送的消息内容 */
							100);		   /* 等待时间 100 Tick */				
						
			}			
			
		}		
		
		/* 触屏-右滑事件 */
		if (EventValue & EVENT_GROUP_FN_TP_HIT)	
		{
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_HIT\r\n");
			
			tp = g_tp;		

			/* 获取当前菜单有多少个项目 */
			item_total = menu_item_total(m_ext.menu);

			/* 保存当前菜单有多少个项目 */
			m_ext.item_total = item_total;			

			/* 通过y坐标确认选中哪个菜单项 */
			if(tp.y && tp.y<60)		item_cursor=0;
			if(tp.y>60 && tp.y<120)	item_cursor=1;
			if(tp.y>120 && tp.y<180)item_cursor=2;
			if(tp.y>180 && tp.y<240)item_cursor=3;
			if(tp.y>240 && tp.y<320)item_cursor=4;
			if(tp.y>320 && tp.y<380)item_cursor=5;	
			if(tp.y>380 && tp.y<420)item_cursor=6;
			if(tp.y>420 && tp.y<480)item_cursor=7;	
			
			if(item_cursor > (item_total-1))
				continue;	

			/* 显示光标 */
			if(item_cursor_last != item_cursor)
			{
				
				m = &m_ext.menu[item_cursor];
				
				item_cursor_last = item_cursor;

				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,0,10,g_lcd_height,WHITE);		
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);	
				
				vTaskDelay(100);
			}			
			
			/* 发送KEY_ENTER按键事件 */
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
		}
		
		/* 触屏-垂直事件 */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_VERTICAL)	
		{
			tp = g_tp;
			
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_VERTICAL\r\n");
			
			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}	
			
			/* 获取当前菜单有多少个项目 */
			item_total = menu_item_total(m_ext.menu);

			/* 保存当前菜单有多少个项目 */
			m_ext.item_total = item_total;			

			/* 通过y坐标确认选中哪个菜单项 */
			if(tp.y && tp.y<60)		item_cursor=0;
			if(tp.y>60 && tp.y<120)	item_cursor=1;
			if(tp.y>120 && tp.y<180)item_cursor=2;
			if(tp.y>180 && tp.y<240)item_cursor=3;
			if(tp.y>240 && tp.y<320)item_cursor=4;
			if(tp.y>320 && tp.y<380)item_cursor=5;	
			if(tp.y>380 && tp.y<420)item_cursor=6;
			if(tp.y>420 && tp.y<480)item_cursor=7;	
			
			if(item_cursor > (item_total-1))
				continue;

			/* 判断选中菜单项是否有变化，若有变化，则重新显示光标，并m指向对应当菜单项内容 */
			if(item_cursor_last != item_cursor)
			{
				
				m = &m_ext.menu[item_cursor];
				
				item_cursor_last = item_cursor;
				
				/* 显示光标 */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,0,10,g_lcd_height,WHITE);		
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);	

				
			}		
			
		}			
		
		if (EventValue & EVENT_GROUP_FN_KEY_UP)
		{
			
			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}		
			
							
			dgb_printf_safe("[app_task_menu] KEY_UP\r\n");
			
			dgb_printf_safe("[app_task_menu] item_total=%d\r\n", menu_item_total(m));
			
			if(item_cursor)
			{
				
				dgb_printf_safe("[app_task_menu] item_cursor1=%d\r\n",item_cursor);
				
				item_cursor--;
				
				dgb_printf_safe("[app_task_menu] item_cursor2=%d\r\n",item_cursor);
				
				/* 项目菜单指向上一个菜单 */
				m = &m_ext.menu[item_cursor];				
				
				/* 显示光标 */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,(item_cursor+1)*60,10,50,WHITE);		
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);
					
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor3=%d\r\n",item_cursor);
				
				/* 若main菜单有同级的左侧菜单 */
				if (m->same_left)
				{
					dgb_printf_safe("[app_task_menu] menu main switch to the left menu\r\n");
					
					item_cursor = 0;

					m_main--;

					m_ext.menu = *m_main;
					m_ext.key_fn = KEY_NONE;
					m_ext.item_cursor = 0;
					m_ext.item_total = menu_item_total(m_ext.menu);

					m = m_ext.menu;

					/* 显示菜单内容 */
					menu_show(&m_ext);
							
				}
			}

			/* 保存当前按键编码 */
			m_ext.key_fn = KEY_UP;

			/* 保存当前项目光标位置值 */
			m_ext.item_cursor = item_cursor;

				
			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
						&beep,		   /* 发送的消息内容 */
						100);		   /* 等待时间 100 Tick */				
		
	
		}
		
		if (EventValue & EVENT_GROUP_FN_KEY_DOWN)
		{
			dgb_printf_safe("[app_task_menu] KEY_DOWN\r\n");
			
			
			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}
			
			/* 获取当前菜单有多少个项目 */
			item_total = menu_item_total(m_ext.menu);

			/* 保存当前菜单有多少个项目 */
			m_ext.item_total = item_total;


			if(item_cursor<(item_total-1))
			{
				item_cursor++;
			
				/* 显示光标 */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,(item_cursor-1)*60,10,50,WHITE);
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);
				
				/* 项目菜单指向下一个菜单 */
				m = &m_ext.menu[item_cursor];	

					
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor6=%d\r\n",item_cursor);
				
				/* 若main菜单有同级的右侧菜单 */
				if (m->same_right)
				{
					dgb_printf_safe("[app_task_menu] menu main switch to the right menu\r\n");
					item_cursor = 0;
					m_main++;

					m_ext.menu = *m_main;
					m_ext.key_fn = KEY_NONE;
					m_ext.item_cursor = 0;
					m_ext.item_total = menu_item_total(m_ext.menu);

					m = m_ext.menu;
					

					/* 显示菜单内容 */
					menu_show(&m_ext);
						
				}
			}	

			/* 保存当前按键编码 */
			m_ext.key_fn = KEY_DOWN;

			/* 保存当前项目光标位置值 */
			m_ext.item_cursor = item_cursor;	
			
				
			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
						&beep,		   /* 发送的消息内容 */
						100);		   /* 等待时间 100 Tick */				
			
		}

		if (EventValue & EVENT_GROUP_FN_KEY_ENTER)
		{
			m_ext.key_fn = KEY_ENTER;

			/* 若有项目功能函数在运行，提示需要返回才能进行项目选择 */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			dgb_printf_safe("[app_task_menu] KEY_ENTER item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_ENTER item name %s\r\n", m->item ? m->item : NULL);
			
			m_ext.item_cursor = item_cursor;
			
			m = &m_ext.menu[item_cursor];

			/* 子菜单有效 */
			if (m->child)
			{
				/* 指向子菜单 */
				m = m->child;

				/* 保存当前菜单 */
				m_ext.menu = m;

				/* 显示菜单内容 */
				menu_show(&m_ext);
				
				/* 复位光标值 */
				item_cursor = 0;	
				

			}

			/* 若没有子菜单,则直接执行功能函数 */
			if (!m->child && m->fun)
			{
				/* 标记有项目功能函数在运行 */
				fun_run = 1;		

				m->fun(&m_ext);
				
				dgb_printf_safe("[app_task_menu] !m->child && m->fun beep\r\n");
						
			}
			
			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
						&beep,		   /* 发送的消息内容 */
						100);		   /* 等待时间 100 Tick */				
			
			
		}

		if (EventValue & EVENT_GROUP_FN_KEY_BACK)
		{
			m_ext.key_fn = KEY_BACK;

			dgb_printf_safe("[app_task_menu] KEY_BACK item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_BACK item name %s\r\n", m->item ? m->item : NULL);

			/* 若子菜单功能函数有效，先执行，主要是挂起对应任务 */
			if (m->fun)
			{
				/* 标记有项目功能函数在运行 */
				fun_run = 1;
			
				m->fun(&m_ext);
				
			}

			/* 父菜单有效 */
			if (m->parent)
			{
				/* 指向父菜单 */
				m = m->parent;

				/* 保存当前菜单 */
				m_ext.menu = m;

				/* 复位光标值 */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : NULL);

				fun_run = 0;

				/* 显示当前菜单 */
				menu_show(&m_ext);
				
					
			}
			
				
			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* 消息队列的句柄 */
						&beep,		   /* 发送的消息内容 */
						100);		   /* 等待时间 100 Tick */				
						
		}		
	}
} 


/**
 * @brief 软件定时器任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 倒数管理、独立看门狗喂狗
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{
	
	/* 统计系统无操作计数值自加1 */
	g_system_no_opreation_cnt++;	
	
	if(g_system_no_opreation_cnt >= 20)
	{
		/* 熄屏 */
		LCD_SAFE
		(
			lcd_display_on(0);
		);
		
		g_system_display_on=0;
	
	}
	
}



/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


void vApplicationTickHook( void )
{

}
