/* ����:��Ƕ.������ */
#include "includes.h"

/* �˵���� */
static menu_t menu_date_time[];

static menu_t menu_temp_humi[];

static menu_t menu_settings[];
static menu_t menu_light[];
static menu_t menu_battery[];
static menu_t menu_set_time[];
static menu_t menu_ble_set_time[];
static menu_t menu_ble_set_date[];

static menu_t menu_step[];
static menu_t menu_sedentary[];

static menu_t menu_heart_blood[];


static menu_t menu_facid[];
static menu_t menu_touchid[];
static menu_t menu_camera[];
static menu_t menu_rfid[];


static void menu_rtc_fun(void* pvParameters);
static void menu_dht_fun(void* pvParameters);
static void menu_light_fun(void* pvParameters);
static void menu_battery_fun(void* pvParameters);
static void menu_set_time_fun(void* pvParameters);
static void menu_step_fun(void* pvParameters);
static void menu_sedentary_fun(void* pvParameters);
static void menu_pulse_fun(void* pvParameters);
static void menu_facid_fun(void* pvParameters);
static void menu_touchid_fun(void* pvParameters);
static void menu_rfid_fun(void* pvParameters);
static void menu_camera_fun(void* pvParameters);



static void item_date_time(void);
static void item_temp_humi(void);

volatile uint8_t nw = 0;

/*	һ���˵�
	1.ʱ��
	2.��ʪ��
	3.����
*/
menu_t menu_main_1[]=
{
	/*  ����         ͼ��  						����   		��         	 ��   	 ��  	��      */
	{"Date time"	,gImage_pic_date1_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_date_time},
	{"Temp humi"	,gImage_temp_humi_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_temp_humi},
	{"Settings"		,gImage_pic_settings_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_settings},	
	{"Games"		,gImage_pic_games_48x48,	NULL,		NULL,	menu_main_2,	NULL,	NULL/*menu_games*/},
	{NULL,NULL,NULL,NULL,NULL,NULL},
};


/*	һ���˵�
	3.�˶�����
	4.����Ѫ��
*/

menu_t menu_main_2[]=
{
	/*  ����         ͼ��,             			����  	   ��   			��           ��  		��      */
	{"Step"			,gImage_step_48x48,			NULL,	menu_main_1,		NULL,		NULL,	menu_step},
	{"Light"		,gImage_light_48x48,		NULL,	menu_main_1,		NULL,		NULL,	menu_light},
	{"Sedentary"	,gImage_sedentary_48x48,	NULL,	menu_main_1,		NULL,		NULL,	menu_sedentary},
	{"HB"			,gImage_heart_blood_48x48,	NULL,	menu_main_1,		NULL,		NULL,	menu_heart_blood},
	
	{NULL,NULL,NULL,NULL,NULL,NULL},
};
/*	һ���˵�
	
	6.�ƹ�
	7.����
	8.��Ϸ
*/


menu_t *menu_main_tbl[]={
	menu_main_1,
	menu_main_2,
//	menu_main_3,	
	NULL,
};

static menu_t menu_date_time[]=
{
	{"-",NULL,menu_rtc_fun,NULL,NULL,menu_main_1,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_temp_humi[]=
{
	{"-",NULL,menu_dht_fun,NULL,NULL,menu_main_1,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_settings[]=
{
	
	{"Battery"		,gImage_battery_48x48,	NULL,			NULL,			NULL,	menu_main_1,	menu_battery},
	{"Set time"		,gImage_ble_48x48,		NULL,			NULL,			NULL,	menu_main_1,	menu_set_time},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

static menu_t menu_light[]=
{
	{"-"			,NULL,			menu_light_fun,			NULL,			NULL,	menu_main_2,			NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_battery[]=
{
	{"-"			,NULL,			menu_battery_fun,		NULL,			NULL,	menu_settings,			NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

static menu_t menu_set_time[]=
{
	{"-",NULL,menu_set_time_fun,NULL,NULL,menu_settings,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

static menu_t menu_heart_blood[]=
{
	{"-",NULL,menu_pulse_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},
};

static menu_t menu_step[]=
{
	{"-",NULL,menu_step_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_sedentary[]=
{
	{"-",NULL,menu_sedentary_fun,NULL,NULL,menu_main_2,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};


static menu_t menu_rfid[]=
{
	{"-",NULL,menu_rfid_fun,NULL,NULL,menu_main_3,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

static menu_t menu_camera[]=
{
	{"-",NULL,menu_camera_fun,NULL,NULL,menu_main_3,NULL},
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL},	
};

/**
 * @brief ����rtc�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details .ʶ��ENTER��,����ݴ������Ŀ���λ��ָ����ȡ����ʱ�仹������,
 * 			 ���ָ�rtc��������У�
 * 			.ʶ��BACK��,�����rtc����
 */
static void menu_rtc_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_rtc_fun] ... \r\n");

	/* ʶ��ENTER��������,��ָ�RTC�������в���ȡ����/ʱ�� */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_rtc_fun] FLAG_DHT_GET_RTC \r\n");
		/* ����Ҫ��ʾ��ͼƬ */	
		LCD_SAFE
		(
//			lcd_fill(0,0,240,280,BLACK);
			lcd_draw_picture(55,30,140,140, gImage_pic_show_date_time_200x163);
		);
			
		/* �ָ�RTC�������� */
		vTaskResume(app_task_rtc_handle);
	}

	/* ʶ��BACK��������,��ֹͣRTC�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		vTaskSuspend(app_task_rtc_handle);
	}
}

// ����ʱ��
static void item_date_time(void)
{
	uint16_t x = 0;
	uint16_t y = 0;
	
	x = 60; lcd_show_chn(x, y+6,6,BLACK,WHITE,16);		//��
	x += 40; lcd_show_chn(x, y+6,7,BLACK,WHITE,16);		//��
	x += 40; lcd_show_chn(x, y+6,8,BLACK,WHITE,16);		//��
	x += 40; lcd_show_chn(x, y+6,9,BLACK,WHITE,16);		//ʱ
	x += 40; lcd_show_chn(x, y+6,10,BLACK,WHITE,16);	//��
}


/**
 * @brief ������ʪ�Ȳ˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details .ʶ��ENTER��,�ָ���ʪ���������У�
 * 			.ʶ��BACK��,������ʪ������
 */
static void menu_dht_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_dht_fun] ...\r\n");

	/* ʶ��ENTER��������,��ָ�DHT�������в���ȡ�¶�/ʪ�� */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_DHT \r\n");
		
		/* ����Ҫ��ʾ��ͼƬ */
		LCD_SAFE
		(					
		
			lcd_draw_picture(60,25,120,120,gImage_waether_180x210);
		);
		
		/* �ָ�DHT�������� */ 
		vTaskResume(app_task_dht_handle);
	}

	/* ʶ��BACK��������,��ֹͣDHT�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* ����DHT���� */
		vTaskSuspend(app_task_dht_handle);
	}	
}


/**
 * @brief ����ƹ����ò˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_light_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_light_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */

		
	}

}


/**
 * @brief ���������ʾ�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_battery_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_battery_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */

		
	}

}


/**
 * @brief �����޸�ʱ�����ڲ˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_set_time_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_set_time_fun] ...\r\n");
	
	//����
	LCD_SAFE(
		/* ���� */
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
		
	);

	/* ʶ��ENTER��������,��ָ�DHT�������в���ȡ�¶�/ʪ�� */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
			dgb_printf_safe("[menu_ble����time]\r\n");
			
			g_rtc_get_what = FLAG_RTC_GET_TIME;
			
			/* ����Ҫ��ʾ��ͼƬ����ʾ��Ϣ */
			LCD_SAFE
			(					
				lcd_draw_picture(65, 50, 120, 120, gImage_pic_set_time_131x131);
			
				lcd_show_chn(10,180,37,BLACK,WHITE,32);		//��
				lcd_show_chn(40,180,38,BLACK,WHITE,32);		//��
				lcd_show_chn(72,180,39,BLACK,WHITE,32);		//��
				lcd_show_chn(104,180,40,BLACK,WHITE,32);	//��
				lcd_show_chn(136,180,59,BLACK,WHITE,32);	//��
				lcd_show_chn(168,180,41,BLACK,WHITE,32);	//��
				lcd_show_chn(200,180,42,BLACK,WHITE,32);	//��
			);
		
		
		if( g_rtc_get_what!=FLAG_RTC_GET_TIME)
			return;		
		vTaskResume(app_task_ble_handle);
	}

	/* ʶ��BACK��������,��ֹͣBLE�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		vTaskSuspend(app_task_ble_handle);
		g_rtc_get_what=FLAG_RTC_GET_NONE;
	}
}


/**
 * @brief ����ǲ���ʾ�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_step_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_step_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_step_fun] FLAG_DHT_GET_Step\r\n");
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
		
		
		/* ����Ҫ��ʾ��ͼƬ */
		LCD_SAFE(
//			lcd_fill(0, 0, 240, 280,BLACK);
			lcd_draw_picture(60, 40, 120, 120, gImage_walk_count_166x100);
		);
		vTaskResume(app_task_mpu6050_handle);
	}
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		/* ����MPU6050���� */
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
		vTaskSuspend(app_task_mpu6050_handle);
	}	
}


/**
 * @brief ���������ʾ�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_sedentary_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_step_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_step_fun] FLAG_DHT_GET_Sedentary\r\n");
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
		
		
		/* ����Ҫ��ʾ��ͼƬ */
		LCD_SAFE(
//			lcd_fill(0, 0, 240, 280,BLACK);
			lcd_draw_picture(60,70,120,120, gImage_walk_count_166x100);
		);
		vTaskResume(app_task_mpu6050_handle);
	}
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		/* ����MPU6050���� */
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
		vTaskSuspend(app_task_mpu6050_handle);
	}	
}


/**
 * @brief ��������Ѫ���˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ʶ��ENTER��,����ݴ������Ŀ���λ��ָ����ȡ����Ѫ��,
 * 			 ���ָ�max30102��������У�
 * 			.ʶ��BACK��,�����max30102����	
 */
static void menu_pulse_fun(void* pvParameters)
{	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_pulse_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		dgb_printf_safe("[menu_pulse_fun] FLAG_DHT_GET_Pulse\r\n");
		
		/* ����Ҫ��ʾ��ͼƬ */
		LCD_SAFE
		(
			lcd_show_chn(60,10,32,BLACK,WHITE,32);
			lcd_show_chn(105,10,33,BLACK,WHITE,32);
			lcd_show_chn(150,10,34,BLACK,WHITE,32);
		
			lcd_draw_picture(60,40,120,120,gImage_pic_in_the_pulse_120x120);
		);

		/* �ָ�max30102�������� */ 
		vTaskResume(app_task_max30102_handle);
	}

	/* ʶ��BACK��������,��ֹͣmax30102�������� */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* ����max30102���� */
		vTaskSuspend(app_task_max30102_handle);
	}	
}


/**
 * @brief ����rfid�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����		
 */
static void menu_rfid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */
	
	
	
	}	
}
/**
 * @brief ����camera�˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_camera_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_camera_fun] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
	
	
	
	}	
}

/**
 * @brief ��������ʶ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_facid_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;


	dgb_printf_safe("[menu_facid] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */


	}
}

/**
 * @brief ����ָ��ʶ��˵���ִ����صĿ���������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ����	
 */
static void menu_touchid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_touchid] ...\r\n");

	/* ʶ��ENTER��������*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* ����Ҫ��ʾ��ͼƬ */


	}
}

/**
 * @brief ��ȡ��ǰ�˵�����Ŀ����
 * @param .menu ָ��ǰ�˵�
 * @retval ��ǰ�˵���Ŀ����
 * @details ��
 */
uint32_t menu_item_total(menu_t *menu)
{
	menu_t *m = menu;

	uint32_t item_count=0;

	while(m->item)
	{
		/* ָ����һ���˵� */
		m++;

		/* ͳ����Ŀ���� */
		item_count++;
	}
		
	return item_count;
}


/**
 * @brief ��ʾ�˵���Ŀ
 * @param .menu_ext ָ��ǰ�˵�
 * @retval ��ǰ�˵���Ŀ����
 * @details ��
 */
void menu_show(menu_ext_t *menu_ext)
{
	uint16_t x;
	uint8_t  y=0;

	menu_ext_t *m_ext = menu_ext;
	menu_t *m = m_ext->menu;
		

	LCD_SAFE
	(
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
	);

	while(1)
	{
		/* ���˵���Ŀ������Ч�� */
		if(m->item == NULL || m->item[0]=='-' )
			break;		

		dgb_printf_safe("[menu_show] m->item is %s \r\n",m->item);

		LCD_SAFE
		(
			lcd_draw_picture(0,y,48,48,m->pic);
			lcd_show_string (80,y+6,m->item,BLACK,WHITE,32,0);	
		);
		
		
		/* ָ����һ���˵� */
		m++;

		/* y������һ�� */
		y+=60;
	}
	
	/* ӵ���Ӳ˵�,��ʾ��� */
	if(m->child)
	{
		m_ext->item_cursor = 0;		
	}	
	
	if(m->item[0]!='-')
	{
		LCD_SAFE
		(
			lcd_fill(g_lcd_width-10,0,10,50,GREY);	
		);	
	}
	dgb_printf_safe("[menu_show] end\r\n");
}
