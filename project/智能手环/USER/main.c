/****************************************************************
*��    ��:���Ķ��켣�����ܽ����ֻ� ��FreeRTOS�ۺϴ���ʵ��-LCD�����汾��(1.69Ӣ�磬����оƬcst816t)
*��    ��:Ī��
*��������:2024/10/30
*˵  ��:	
	1.��ǰ����淶���ṹ������������ʵ����ģ�黯��̣�һ���������һ��Ӳ����
	  �ô��������á�FreeRTOS�ۺϴ���ʵ��-OLED�汾�����ص㽫OLED��ʾ�޸�ΪLCD��ʾ
	2.�˵����ӦӲ��ģ�鹦��
		�˵���1ҳ
			.ʱ����ʾ 		[��ʵ��]
			.��ʪ����ʾ  	[��ʵ��]
			.���ã��ƹ⡢������ʾ������ʱ�䣩	[ʵ��������ʱ��]	
			
		�˵���2ҳ
			.��������		[��ʵ��]
			.��������		[��ʵ��]
			.����Ѫ����ʾ  	[��ʵ��]
			
		�˵���3ҳ
			.GAME			[�п���ʵ��]
*����˵��:
	1.��������
		.����1�밴��2���˵����ѡ��
		.����3������˵���
		.����4���˳��˵���
	2.�ɵ�����
		.��ťʽѡ���Ӧ�Ĳ˵�������iPod
	3.������
		.ִ�ж�Ӧ�Ĳ�������һ��ʾ��
	4.�˵���ʵ��
		.�༶�˵��ķ���
	5.�Զ�Ϩ��
		.���޲�������һ���Զ�Ϩ��
		.�������ⰴ��������Ļ
	6.����
		.����������˵���
		.�ϻ�/�»����˵�������ѡ��
		.�һ����л�����һ��ͬ���˵�
		.�󻬣��л���ǰһ��ͬ���˵�
		
*ע������:	

	1.LCD
	��1����ǰ����ֻ��ʹ��ģ��SPI��ԭ���Ӳ��ʹ����9bit SPI�����ʽ(D/C xxxx xxxx��bit8Ϊ����/���� bit7~bit0Ϊ����)
	     ��STM32��Ӳ��SPIֻ��֧��8bit/16bit�Ĵ����ʽ
	2.������
	��1�����ڸ���Ļ��С������ʱ���ܴ�����
	��2���ô���оƬ�����⣬һֱ��ѹ��Ļ���᲻�ϴ����жϣ�����һֱ�͵�ƽ��������������оƬ������
	��3����鴥���Ƿ��뿪��Ļ����Ҫ��ȡ�ô��������ֵ���ֽ�[7:6]ȥ�ж�
		
*�ؼ�������
	1.app_task_menu���ļ�·����main.c
		.����Բ˵���ѡ��/����/�˳��Ĺ���
		.��ʾ���ָʾ��ǰ�˵���
		
	2.menu_show,�ļ�·����menu.c
		.��ʾ�˵���ͼ��
		.��ʾ�˵�������
		
*�ؼ��������ͣ�	
	1.menu_t���ļ�·����menu.h
		.��ʽ�洢ˮƽ��/�Ҳ˵�����/�Ӳ˵�
		
*�ؼ��꣺	
	1.DEBUG_PRINTF_EN���ļ�·����includes.h
		.ʹ����ͨ������1���������Ϣ
		
	2.LCD_SAFE���ļ�·����includes.h
		.ʹ�û���������LCD����
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


/* �����ʱ����� */
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
volatile uint32_t g_ble_status = FLAG_BLE_STATUS_NONE;//��������״̬
volatile uint32_t g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
volatile uint32_t g_system_no_opreation_cnt = 0;
volatile uint32_t g_system_display_on = 1;
volatile uint32_t ble_connect_get = 0;


//max30102����ı���
volatile uint32_t aun_ir_buffer[500]; //IR LED sensor data
volatile int32_t n_ir_buffer_length;    //data length
volatile uint32_t aun_red_buffer[500];    //Red LED sensor data
volatile int32_t n_sp02; //SPO2 value
volatile int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
volatile int32_t n_heart_rate;   //heart rate value
volatile int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
volatile uint8_t uch_dummy;

tp_t g_tp;

/* ��Ϣ���о�� */
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_usart;

// ���������
TaskHandle_t app_task_ble_status_handle = NULL;

/* �¼���־���� */
EventGroupHandle_t g_event_group;

/* �������ź������ */
SemaphoreHandle_t g_mutex_printf;
SemaphoreHandle_t g_mutex_lcd;

/* ����*/ 
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


/* �����ʱ�� */
static void soft_timer_callback(TimerHandle_t pxTimer);

void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN

	va_list args;
	va_start(args, format);

	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

/* �����б� */
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
	/* ����ϵͳ�ж����ȼ�����4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);									
	
	/* ��ʼ������1 */
	usart_init(115200);     							

	/* ����app_task_init���� */
	xTaskCreate((TaskFunction_t )app_task_init,  	/* ������ں��� */
			  (const char*    )"app_task_init",		/* �������� */
			  (uint16_t       )512,  				/* ����ջ��С */
			  (void*          )NULL,				/* ������ں������� */
			  (UBaseType_t    )5, 					/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_init_handle);/* ������ƿ�ָ�� */ 

			  
	/* ����������� */
	vTaskStartScheduler(); 
	

	printf("none run here...\r\n");
			  
	while(1);

}

void lcd_startup_info(void)
{
	uint16_t x=0;
	
	LCD_SAFE(
			
		/* ���� */
		lcd_clear(WHITE);
		
		/* ��ʾ������ */
		lcd_draw_picture((g_lcd_width-95)/2,(g_lcd_height-50)/2-15,100,100,gImage_logo_60x60);
	
		x=(g_lcd_width-4*37)/2;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,16,BLACK,WHITE,32);	// ��
		lcd_show_chn(x+20,(g_lcd_height-60)/2-50,28,BLACK,WHITE,32);// ��
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,24,BLACK,WHITE,32);		// Ī
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,25,BLACK,WHITE,32);	// ��
		lcd_show_chn(x+10,(g_lcd_height-60)/2-50,29,BLACK,WHITE,32);// ��
	
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,57,BLACK,WHITE,32);	// 
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,55,BLACK,WHITE,32);	// ��
		lcd_show_chn(x,(g_lcd_height-60)/2-50,30,BLACK,WHITE,32);	// ��
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2+100,26,BLACK,WHITE,32);		// ��
		
		x+=20;
		lcd_show_chn(x,(g_lcd_height-60)/2-90,56,BLACK,WHITE,32);	// ��
		lcd_show_chn(x-10,(g_lcd_height-60)/2-50,31,BLACK,WHITE,32);// ��
	
		x=0;
		while(x<g_lcd_width)
		{
			/* ������ */
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
	
	/* ������Ϣ���� */
	g_queue_beep  	= xQueueCreate(QUEUE_BEEP_LEN, 	sizeof(beep_t));
	
	g_queue_usart = xQueueCreate(5, 128);
	
	/* �����������ź��� */
	g_mutex_printf 	= xSemaphoreCreateMutex();		  
	g_mutex_lcd 	= xSemaphoreCreateMutex();	

	/* �����¼���־�� */
	g_event_group = xEventGroupCreate();		
	
	/* lcd��ʼ�� */ 
	lcd_init();
	
	/* beep��ʼ�� */
	beep_init();

	/* ������ʼ�� */
	key_init();	
	
	/* adc��ʼ�� */
	adc_init();

	/* rtc��ʼ�� */
	rtc_init();	
	
	/* ��ʪ��ģ���ʼ�� */
	dht11_init();	
	
	/* ����оƬ��ʼ�� */
	tp_init();	

	/* ����Ѫ��ģ���ʼ�� */
	max30102_init();
	
	/*����ģ���ʼ��*/
	blue_tooth_init(9600);
	
	/* MPU6050��ʼ�� */
	MPU_Init();
	
	taskENTER_CRITICAL();
	

	/* �����õ������� */
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* ������ں��� */
					task_tbl[i].pcName,			/* �������� */
					task_tbl[i].usStackDepth,	/* ����ջ��С */
					task_tbl[i].pvParameters,	/* ������ں������� */
					task_tbl[i].uxPriority,		/* ��������ȼ� */
					task_tbl[i].pxCreatedTask); /* ������ƿ�ָ�� */
		
		 // ��������״̬��ʾ����
		xTaskCreate((TaskFunction_t)app_task_ble_status,
                (const char *)"app_task_ble_status",
                256,           // ջ��С
                NULL,          // �������
                2,             // ���ȼ�
                &app_task_ble_status_handle);
		i++;
	}
	
	taskEXIT_CRITICAL();
	
	/* �������������ʱ�� */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* ��ʱ������ 1000(tick) */
									 (UBaseType_t)pdTRUE, /* ����ģʽ */
									 (void *)1,			  /* Ϊÿ����ʱ������һ��������ΨһID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* �������������ʱ�� */
	xTimerStart(soft_timer_Handle, 0);	
									 
//	vTaskSuspend(app_task_ble_handle);
	
	
	/* ��ʾ���˵� */
	
	lcd_startup_info();	

	m_ext.menu=menu_main_1;
	
	menu_show(&m_ext);
	

	vTaskDelete(NULL);
}   

/**
 * @brief ��������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 */
static void app_task_key(void *pvParameters)
{
	EventBits_t EventValue;

	
	
	dgb_printf_safe("[app_task_key] create success\r\n");

	for (;;)
	{
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* ��ʱ���� */
		vTaskDelay(20);
		
		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;		
		
		if(g_system_display_on==0)
		{
			lcd_display_on(1);
			g_system_display_on=1;
			continue;
			
		}
		
	
			

		if (EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			/* ��ֹEXTI0�����ж� */
			NVIC_DisableIRQ(EXTI0_IRQn);

			/* ȷ���ǰ��� */
			if (PAin(0) == 0)
			{

				dgb_printf_safe("[app_task_key] S1 Press\r\n");

				/* ����KEY_UP�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_DOWN);


			}
			
			while(PAin(0)==0)
					vTaskDelay(10);
			
			/* ����EXTI0�����ж� */
			NVIC_EnableIRQ(EXTI0_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			/* ��ֹEXTI2�����ж� */
			NVIC_DisableIRQ(EXTI2_IRQn);

			if (PEin(2) == 0)
			{
				dgb_printf_safe("[app_task_key] S2 Press\r\n");

				/* ����KEY_DOWN�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_UP);
							
			}
			
			while(PEin(2)==0)
					vTaskDelay(10);			

			/* ����EXTI2�����ж� */
			NVIC_EnableIRQ(EXTI2_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			/* ��ֹEXTI3�����ж� */
			NVIC_DisableIRQ(EXTI3_IRQn);

			if (PEin(3) == 0)
			{
				dgb_printf_safe("[app_task_key] S3 Press\r\n");
			
				/* ����KEY_ENTER�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
				
			}
			
			while(PEin(3)==0)
					vTaskDelay(10);			

			/* ����EXTI3�����ж� */
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			/* ��ֹEXTI4�����ж� */
			NVIC_DisableIRQ(EXTI4_IRQn);

			if (PEin(4) == 0)
			{

				dgb_printf_safe("[app_task_key] S4 Press\r\n");

				/* ����KEY_BACK�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);				
				
			}
			
			while(PEin(4)==0)
					vTaskDelay(10);				

			/* ����EXTI4�����ж� */
			NVIC_EnableIRQ(EXTI4_IRQn);
		}
	}
}

/**
 * @brief ����������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
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
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_TP,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;		
		
		if(g_system_display_on==0)
		{
			lcd_display_on(1);
			g_system_display_on=1;
			continue;
		}
		
		
		
		/* ��ֹTP�����ж� */
		NVIC_DisableIRQ(EXTI9_5_IRQn);
		
		if ((EventValue & EVENT_GROUP_TP))
		{
			
			while(1)
			{
				tp_sta=tp_read(&screen_x,&screen_y);
				
				g_tp.x=screen_x;			
				g_tp.y=screen_y;
				
				
				//��ȡ�����ָ������Ϊ0������ѭ��
				if(tp_finger_num_get() == 0)
						break;		
				
				if(tp_sta==0x01)
				{
					
					/* �����¼���־���еĴ�ֱ�����¼� */
					EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_VERTICAL);
					
					tp_slide_event=TP_SLIDE_DOWN;			
				}			
				
				if(tp_sta==0x02)
				{
					/* �����¼���־���еĴ�ֱ�����¼� */
					EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_VERTICAL);
					
					tp_slide_event=TP_SLIDE_UP;			
				}				
				
				vTaskDelay(10);
			}

		}	

		
		if(tp_sta==0x03)
		{
			/* �����¼���־���е����󻬶��¼� */
			EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_LEFT);
			
			tp_slide_event=TP_SLIDE_LEFT;			
		}	


		if(tp_sta==0x04)
		{
			/* �����¼���־���е����һ����¼� */
			EventValue=xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_SLIDE_RIGHT);
			
			tp_slide_event=TP_SLIDE_RIGHT;		
		}

		
		if(tp_sta==0x05)
		{
			/* ���ִ���¼� */
			xEventGroupSetBits(g_event_group,EVENT_GROUP_FN_TP_HIT);			
		}


		printf("tp_sta=%02X g_system_no_opreation_cnt=%d\r\n",tp_sta,g_system_no_opreation_cnt);
		
		/* ����TP�����ж� */
		NVIC_EnableIRQ(EXTI9_5_IRQn);
	}
}

/**
 * @brief adc����
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ͨ���ɵ������ȡ�ĵ�ѹֵ������ת�̲˵��Ŀ���
 */
static void app_task_adc(void *pvParameters)
{
	int32_t adc_vol_last = 0;
	int32_t adc_vol = 0;
	int32_t result;

	/* ��ȡ��ǰ��ѹ��ֵ */
	adc_vol = adc_voltage_get();
	adc_vol_last = adc_vol;

	for (;;)
	{
		/* ��ȡ��ѹֵ */
		adc_vol = adc_voltage_get();

		result = adc_vol - adc_vol_last;

		if (result > 200 || result < -200)
		{

			/* ����KEY_DOWN/KEY_UP�����¼� */
			xEventGroupSetBits(g_event_group, result > 0 ? EVENT_GROUP_FN_KEY_UP : EVENT_GROUP_FN_KEY_DOWN);

			dgb_printf_safe("[app_task_adc] adc_vol_last=%d adc_vol=%d\r\n", adc_vol_last, adc_vol);

			adc_vol_last = adc_vol;
		}

		vTaskDelay(200);
	}
}

/**
 * @brief ����������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ������Ϣ���п��Ʒ�������
 */
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_beep] create success\r\n");

	for (;;)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* ��Ϣ���еľ�� */
								&beep,			/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
			continue;

		dgb_printf_safe("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);
		
		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;				

		/* ���������Ƿ���Ҫ�������� */
		if (beep.duration)
		{
			BEEP(beep.sta);

			while (beep.duration--)
				vTaskDelay(10);

			/* ������״̬��ת */
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
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);		
		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;			
		
		if (EventValue & EVENT_GROUP_RTC_WAKEUP)
		{
			/* RTC_GetTime����ȡʱ�� */
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
			/* RTC_GetDate����ȡ���� */
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
			memset(time_buf, 0, sizeof time_buf);
			memset(second_buf, 0, sizeof second_buf);
			
			memset(year_buf, 0, sizeof year_buf);
			memset(month_buf, 0, sizeof month_buf);
			memset(date_buf, 0, sizeof date_buf);

			/* ��ʽ���ַ��� */
			sprintf((char *)time_buf, "%02d:%02d", RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes);
			sprintf((char *)second_buf, "%02d", RTC_TimeStructure.RTC_Seconds);

			/* ��ʽ���ַ��� */
			sprintf((char *)year_buf, "20%02d", RTC_DateStructure.RTC_Year);
			sprintf((char *)month_buf, "%02d", RTC_DateStructure.RTC_Month);
			sprintf((char *)date_buf, "%02d", RTC_DateStructure.RTC_Date);
			
			LCD_SAFE(
				lcd_show_string(70,180,time_buf,BLACK,WHITE,32,0);
				
				lcd_show_string(160,170,second_buf,BLACK,WHITE,24,0);
				
				
				lcd_show_string(30-20,240,year_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+50-20, 240,6,BLACK,WHITE,24);	// ��
				
				lcd_show_string(30+75-20,240,month_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+100-20, 240,7,BLACK,WHITE,24);	// ��
				
				lcd_show_string(30+125-20,240,date_buf,BLACK,WHITE,24,0);
				lcd_show_chn(30+150-20, 240,8,BLACK,WHITE,24);	// ��
				
				
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
		
		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;				

		if (rt==0)
		{
			memset(temp_buf, 0, sizeof temp_buf);
			memset(humi_buf, 0, sizeof humi_buf);
			
			sprintf((char *)temp_buf, "%02d.%d", dht11_data[2], dht11_data[3]);
			sprintf((char *)humi_buf, "%02d.%d%%", dht11_data[0], dht11_data[1]);
			
			/* ��ʾ�¶�/ʪ�� */
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
		// ���ڽ�������
		xQueueReceive(g_queue_usart, buf, portMAX_DELAY);

		printf("%s\r\n", buf);

		// ����ʱ��
		if (g_rtc_get_what == FLAG_RTC_GET_TIME || g_ble_status == FLAG_BLE_STATUS_CONNECT)
		{
			// ��TIME SET-10-20-30#��
			if (strstr((const char *)buf, "TIME"))
			{
				p = strtok((char *)buf, "-");
				// Сʱ10
				p = strtok(NULL, "-");
				hours = atoi(p);
				// ����20
				p = strtok(NULL, "-");
				minutes = atoi(p);
				// ��30
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
						/* ���� */
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
			
			// ��DATE SET-2023-5-25-4#��
			if (strstr((const char *)buf, "DATE"))
			{
				p = strtok((char *)buf, "-");
				p = strtok(NULL, "-"); // 2023
				year = atoi(p);
				// ��ȡ�·�
				p = strtok(NULL, "-");
				month = atoi(p);

				// ��ȡ����
				p = strtok(NULL, "-");
				day = atoi(p);
				// printf("p=%s",p);

				// ��ȡ���ڼ�
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
						/* ���� */
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


// ��������״̬��ʾ����
static void app_task_ble_status(void *pvParameters)
{
    for (;;)
    {
        // �����������״̬
        ble_connect_get = ble_connect_sta_get();
        if (ble_connect_get == FLAG_BLE_STATUS_CONNECT)
        {
            // ��ʾ����ͼ������Ļ�Ĺ̶�λ��
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
		

        // ��ʱһ��ʱ�䣬����Ƶ��ˢ�£�����500���룩
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
		
		   ��ǰ100������ת�����洢���У��������400�������Ƶ�����
		*/
		
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            /* update the signal min and max 
			   �����ź���Сֵ�����ֵ
			*/
			
            if(un_min>aun_red_buffer[i])
				un_min=aun_red_buffer[i];
			
            if(un_max<aun_red_buffer[i])
				un_max=aun_red_buffer[i];
        }
		
		/* take 100 sets of samples before calculating the heart rate 
		
		   �ڼ�������֮ǰ�ɼ�100������
		*/
		
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
			
            while(MAX30102_INT==1);
			
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			
			/* ���ֵ�Ի��ʵ������ */
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
		
		/* �������ʺ�Ѫ�����Ͷ� */
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		
		/* ͨ��UART�������ͼ��������͵��ն˳��� */
		if((ch_hr_valid == 1)&& (n_heart_rate>=60) && (n_heart_rate<100))
		{

			dgb_printf_safe("����=%d\r\n", n_heart_rate);
			
			sprintf(buff, ":%d", n_heart_rate);
			
			LCD_SAFE(
				lcd_show_chn(60,160,16,BLACK,WHITE,32);
				lcd_show_chn(92,160,17,BLACK,WHITE,32);
				
				/* ��ʾ���� */
				lcd_show_string(130,160,buff,BLACK,WHITE,32,0);
			);
			
			memset(buff, 0, sizeof buff);
		}

		
		if((ch_spo2_valid ==1)&& (n_sp02>=95) && (n_sp02<100))
		{
			dgb_printf_safe("Ѫ��Ũ��=%d\r\n", n_sp02);

			sprintf(bufff, ":%d", n_sp02);
			
			LCD_SAFE(
				lcd_show_chn(30,200,18,BLACK,WHITE,32);
				lcd_show_chn(62,200,19,BLACK,WHITE,32);
				
				lcd_show_chn(94,200,35,BLACK,WHITE,32);
				lcd_show_chn(126,200,21,BLACK,WHITE,32);
				
				/* ��ʾѪ��Ũ�� */
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
	float pitch, roll, yaw; /* ŷ���� */

	dgb_printf_safe("[app_task_mpu6050] create success and suspend self \r\n");

	//vTaskSuspend(NULL);

	dgb_printf_safe("mpu6050����������......r\n");

	while(mpu_dmp_init())
	{
		delay_ms(100);
	}
	
	/* ���ò�����ֵΪ0*/
	while(dmp_set_pedometer_step_count(0))
	{
		delay_ms(100);
	}

	for (;;)
	{
		/* ��ȡ���� */
		dmp_get_pedometer_step_count(&step_count);
		
		//dgb_printf_safe("mpu6050== %dr\n",step_count);
		//dgb_printf_safe("[INFO] ��ǰ����:%ld ��ǰ����:%ld\r\n",step_count,step_count_last);
		
		step_count_last=step_count;
		
		sprintf((char *)buf, ":%d",step_count);
		// dgb_printf_safe("%d\r\n",g_mpu6050_get_what);
		if (g_mpu6050_get_what == FLAG_MPU6050_GET_STEP)
		{
			/* ��ǰ���� */
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
		
			/* ��ָ��������û�����ߣ����������¼� */
			if(t>=100)
			{
				/* ��鲽���ı仯 */
				if((step_count - step_count_last) < 5)
				{
					/* �����仯���������þ����¼���־λΪ1 */
					sedentary_event=1;
				}
				
				dgb_printf_safe("[INFO] ��ǰ����:%ld ��ǰ����:%ld\r\n",step_count,step_count_last);
				
				step_count_last=step_count;
				
				t=0;
			}
			
			if(sedentary_event)
			{
				sedentary_event=0;
				
				dgb_printf_safe("[INFO] �������ã���վ��������...\r\n");
				
				
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
		
		/* ̧������ */
		if(0 == mpu_dmp_get_data(&pitch, &roll, &yaw))
		{
			/* ��ʱһ�� */
			delay_ms(200);
			
			if(roll > 15 && !g_system_display_on)
			{
				/* �����ٽ��� */
				taskENTER_CRITICAL();
				
				g_system_display_on=1;

				/* ����ͳ��ϵͳ�޲�������ֵ */
				g_system_no_opreation_cnt=0;

				/* �˳��ٽ��� */
				taskEXIT_CRITICAL();

				dgb_printf_safe("̧������\r\n");

				/* ����OLED�� */
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
	
	uint32_t item_total;			//��¼��ǰ�˵��ж��ٸ���Ŀ
	uint32_t item_cursor_last = 0;	//��¼��ǰ�˵���ǰָ���ĸ���Ŀ
	uint32_t item_cursor = 0;		//��¼��ǰ�˵�ָ���ĸ���Ŀ
	uint32_t fun_run = 0;			//��¼�Ƿ�����Ŀ���ܺ�����ִ��

	menu_ext_t m_ext;

	menu_t *m;

	/* ͨ���������ݻ�ȡ���˵�ҳָ�� */
	menu_t **m_main = (menu_t **)pvParameters;		
	

	
	/* ����m_extָ��ĵ�ǰ�˵�����ز��� */
	m_ext.menu = *m_main;						
	m_ext.key_fn = KEY_NONE;
	m_ext.item_cursor = item_cursor;
	m_ext.item_total = menu_item_total(m_ext.menu);	
	
	
	/* ���ü�¼�˵�ָ��m */
	m = m_ext.menu;	
	
	dgb_printf_safe("[app_task_menu] create success\r\n");
	
	for(;;)
	{
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
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
		
		/* ����ͳ��ϵͳ�޲�������ֵ */
		g_system_no_opreation_cnt = 0;

		/* ����-���¼� */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_LEFT)	
		{
			
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_LEFT \r\n");
			
			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				/* ����KEY_BACK�����¼� */
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_BACK);		

				continue;
			}	
			
			/* ��main�˵���ͬ�������˵� */
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

				/* ��ʾ�˵����� */
				menu_show(&m_ext);
				
				/* ��һ��ʾ�� */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
							&beep,		   /* ���͵���Ϣ���� */
							100);		   /* �ȴ�ʱ�� 100 Tick */				
				
										
			}
			
			/* ���˵���Ч */
			if (m->parent)
			{
				/* ָ�򸸲˵� */
				m = m->parent;

				/* ���浱ǰ�˵� */
				m_ext.menu = m;

				/* ��λ���ֵ */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : NULL);

				fun_run = 0;

				/* ��ʾ��ǰ�˵� */
				menu_show(&m_ext);
				
				/* ��һ��ʾ�� */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
							&beep,		   /* ���͵���Ϣ���� */
							100);		   /* �ȴ�ʱ�� 100 Tick */				
				
							
			}			
		}
		
		/* ����-�һ��¼� */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_RIGHT)	
		{
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_RIGHT \r\n");
			
			/* ��main�˵���ͬ�����Ҳ�˵� */
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
				

				/* ��ʾ�˵����� */
				menu_show(&m_ext);
				
				
				/* ��һ��ʾ�� */
				beep.sta = 1;
				beep.duration = 1;

				xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
							&beep,		   /* ���͵���Ϣ���� */
							100);		   /* �ȴ�ʱ�� 100 Tick */				
						
			}			
			
		}		
		
		/* ����-�һ��¼� */
		if (EventValue & EVENT_GROUP_FN_TP_HIT)	
		{
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_HIT\r\n");
			
			tp = g_tp;		

			/* ��ȡ��ǰ�˵��ж��ٸ���Ŀ */
			item_total = menu_item_total(m_ext.menu);

			/* ���浱ǰ�˵��ж��ٸ���Ŀ */
			m_ext.item_total = item_total;			

			/* ͨ��y����ȷ��ѡ���ĸ��˵��� */
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

			/* ��ʾ��� */
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
			
			/* ����KEY_ENTER�����¼� */
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FN_KEY_ENTER);
		}
		
		/* ����-��ֱ�¼� */
		if (EventValue & EVENT_GROUP_FN_TP_SLIDE_VERTICAL)	
		{
			tp = g_tp;
			
			dgb_printf_safe("[app_task_menu] EVENT_GROUP_FN_TP_SLIDE_VERTICAL\r\n");
			
			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}	
			
			/* ��ȡ��ǰ�˵��ж��ٸ���Ŀ */
			item_total = menu_item_total(m_ext.menu);

			/* ���浱ǰ�˵��ж��ٸ���Ŀ */
			m_ext.item_total = item_total;			

			/* ͨ��y����ȷ��ѡ���ĸ��˵��� */
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

			/* �ж�ѡ�в˵����Ƿ��б仯�����б仯����������ʾ��꣬��mָ���Ӧ���˵������� */
			if(item_cursor_last != item_cursor)
			{
				
				m = &m_ext.menu[item_cursor];
				
				item_cursor_last = item_cursor;
				
				/* ��ʾ��� */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,0,10,g_lcd_height,WHITE);		
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);	

				
			}		
			
		}			
		
		if (EventValue & EVENT_GROUP_FN_KEY_UP)
		{
			
			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
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
				
				/* ��Ŀ�˵�ָ����һ���˵� */
				m = &m_ext.menu[item_cursor];				
				
				/* ��ʾ��� */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,(item_cursor+1)*60,10,50,WHITE);		
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);
					
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor3=%d\r\n",item_cursor);
				
				/* ��main�˵���ͬ�������˵� */
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

					/* ��ʾ�˵����� */
					menu_show(&m_ext);
							
				}
			}

			/* ���浱ǰ�������� */
			m_ext.key_fn = KEY_UP;

			/* ���浱ǰ��Ŀ���λ��ֵ */
			m_ext.item_cursor = item_cursor;

				
			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
						&beep,		   /* ���͵���Ϣ���� */
						100);		   /* �ȴ�ʱ�� 100 Tick */				
		
	
		}
		
		if (EventValue & EVENT_GROUP_FN_KEY_DOWN)
		{
			dgb_printf_safe("[app_task_menu] KEY_DOWN\r\n");
			
			
			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}
			
			/* ��ȡ��ǰ�˵��ж��ٸ���Ŀ */
			item_total = menu_item_total(m_ext.menu);

			/* ���浱ǰ�˵��ж��ٸ���Ŀ */
			m_ext.item_total = item_total;


			if(item_cursor<(item_total-1))
			{
				item_cursor++;
			
				/* ��ʾ��� */
				LCD_SAFE
				(
					lcd_fill(g_lcd_width-10,(item_cursor-1)*60,10,50,WHITE);
					lcd_fill(g_lcd_width-10,item_cursor*60,10,50,GREY);
				);
				
				/* ��Ŀ�˵�ָ����һ���˵� */
				m = &m_ext.menu[item_cursor];	

					
			}
			else
			{
				dgb_printf_safe("[app_task_menu] item_cursor6=%d\r\n",item_cursor);
				
				/* ��main�˵���ͬ�����Ҳ�˵� */
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
					

					/* ��ʾ�˵����� */
					menu_show(&m_ext);
						
				}
			}	

			/* ���浱ǰ�������� */
			m_ext.key_fn = KEY_DOWN;

			/* ���浱ǰ��Ŀ���λ��ֵ */
			m_ext.item_cursor = item_cursor;	
			
				
			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
						&beep,		   /* ���͵���Ϣ���� */
						100);		   /* �ȴ�ʱ�� 100 Tick */				
			
		}

		if (EventValue & EVENT_GROUP_FN_KEY_ENTER)
		{
			m_ext.key_fn = KEY_ENTER;

			/* ������Ŀ���ܺ��������У���ʾ��Ҫ���ز��ܽ�����Ŀѡ�� */
			if (fun_run)
			{
				dgb_printf_safe("[app_task_menu] menu fun is running,please press back\r\n");

				continue;
			}

			dgb_printf_safe("[app_task_menu] KEY_ENTER item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_ENTER item name %s\r\n", m->item ? m->item : NULL);
			
			m_ext.item_cursor = item_cursor;
			
			m = &m_ext.menu[item_cursor];

			/* �Ӳ˵���Ч */
			if (m->child)
			{
				/* ָ���Ӳ˵� */
				m = m->child;

				/* ���浱ǰ�˵� */
				m_ext.menu = m;

				/* ��ʾ�˵����� */
				menu_show(&m_ext);
				
				/* ��λ���ֵ */
				item_cursor = 0;	
				

			}

			/* ��û���Ӳ˵�,��ֱ��ִ�й��ܺ��� */
			if (!m->child && m->fun)
			{
				/* �������Ŀ���ܺ��������� */
				fun_run = 1;		

				m->fun(&m_ext);
				
				dgb_printf_safe("[app_task_menu] !m->child && m->fun beep\r\n");
						
			}
			
			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
						&beep,		   /* ���͵���Ϣ���� */
						100);		   /* �ȴ�ʱ�� 100 Tick */				
			
			
		}

		if (EventValue & EVENT_GROUP_FN_KEY_BACK)
		{
			m_ext.key_fn = KEY_BACK;

			dgb_printf_safe("[app_task_menu] KEY_BACK item cursor=%d\r\n", item_cursor);
			dgb_printf_safe("[app_task_menu] KEY_BACK item name %s\r\n", m->item ? m->item : NULL);

			/* ���Ӳ˵����ܺ�����Ч����ִ�У���Ҫ�ǹ����Ӧ���� */
			if (m->fun)
			{
				/* �������Ŀ���ܺ��������� */
				fun_run = 1;
			
				m->fun(&m_ext);
				
			}

			/* ���˵���Ч */
			if (m->parent)
			{
				/* ָ�򸸲˵� */
				m = m->parent;

				/* ���浱ǰ�˵� */
				m_ext.menu = m;

				/* ��λ���ֵ */
				item_cursor = 0;
				m_ext.item_cursor = 0;

				dgb_printf_safe("[app_task_menu] m->parent item cursor=%d\r\n", item_cursor);
				dgb_printf_safe("[app_task_menu] m->parent item name %s\r\n", m->item ? m->item : NULL);

				fun_run = 0;

				/* ��ʾ��ǰ�˵� */
				menu_show(&m_ext);
				
					
			}
			
				
			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 1;

			xQueueSend( g_queue_beep,  /* ��Ϣ���еľ�� */
						&beep,		   /* ���͵���Ϣ���� */
						100);		   /* �ȴ�ʱ�� 100 Tick */				
						
		}		
	}
} 


/**
 * @brief �����ʱ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ���������������Ź�ι��
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{
	
	/* ͳ��ϵͳ�޲�������ֵ�Լ�1 */
	g_system_no_opreation_cnt++;	
	
	if(g_system_no_opreation_cnt >= 20)
	{
		/* Ϩ�� */
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
