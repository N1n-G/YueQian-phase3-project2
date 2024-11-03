/* 作者:粤嵌.温子祺 */
#include "includes.h"

/* 菜单相关 */
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

/*	一级菜单
	1.时间
	2.温湿度
	3.设置
*/
menu_t menu_main_1[]=
{
	/*  名字         图标  						函数   		左         	 右   	 父  	子      */
	{"Date time"	,gImage_pic_date1_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_date_time},
	{"Temp humi"	,gImage_temp_humi_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_temp_humi},
	{"Settings"		,gImage_pic_settings_48x48,	NULL,		NULL,	menu_main_2,	NULL,	menu_settings},	
	{"Games"		,gImage_pic_games_48x48,	NULL,		NULL,	menu_main_2,	NULL,	NULL/*menu_games*/},
	{NULL,NULL,NULL,NULL,NULL,NULL},
};


/*	一级菜单
	3.运动步数
	4.心率血氧
*/

menu_t menu_main_2[]=
{
	/*  名字         图标,             			函数  	   左   			右           父  		子      */
	{"Step"			,gImage_step_48x48,			NULL,	menu_main_1,		NULL,		NULL,	menu_step},
	{"Light"		,gImage_light_48x48,		NULL,	menu_main_1,		NULL,		NULL,	menu_light},
	{"Sedentary"	,gImage_sedentary_48x48,	NULL,	menu_main_1,		NULL,		NULL,	menu_sedentary},
	{"HB"			,gImage_heart_blood_48x48,	NULL,	menu_main_1,		NULL,		NULL,	menu_heart_blood},
	
	{NULL,NULL,NULL,NULL,NULL,NULL},
};
/*	一级菜单
	
	6.灯光
	7.电量
	8.游戏
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
 * @brief 进入rtc菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details .识别到ENTER键,则根据传入的项目光标位置指定获取的是时间还是日期,
 * 			 并恢复rtc任务的运行；
 * 			.识别到BACK键,则挂起rtc任务。
 */
static void menu_rtc_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_rtc_fun] ... \r\n");

	/* 识别到ENTER按键编码,则恢复RTC任务运行并获取日期/时间 */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_rtc_fun] FLAG_DHT_GET_RTC \r\n");
		/* 设置要显示的图片 */	
		LCD_SAFE
		(
//			lcd_fill(0,0,240,280,BLACK);
			lcd_draw_picture(55,30,140,140, gImage_pic_show_date_time_200x163);
		);
			
		/* 恢复RTC任务运行 */
		vTaskResume(app_task_rtc_handle);
	}

	/* 识别到BACK按键编码,则停止RTC任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		vTaskSuspend(app_task_rtc_handle);
	}
}

// 设置时间
static void item_date_time(void)
{
	uint16_t x = 0;
	uint16_t y = 0;
	
	x = 60; lcd_show_chn(x, y+6,6,BLACK,WHITE,16);		//日
	x += 40; lcd_show_chn(x, y+6,7,BLACK,WHITE,16);		//期
	x += 40; lcd_show_chn(x, y+6,8,BLACK,WHITE,16);		//和
	x += 40; lcd_show_chn(x, y+6,9,BLACK,WHITE,16);		//时
	x += 40; lcd_show_chn(x, y+6,10,BLACK,WHITE,16);	//间
}


/**
 * @brief 进入温湿度菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details .识别到ENTER键,恢复温湿度任务运行；
 * 			.识别到BACK键,挂起温湿度任务
 */
static void menu_dht_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	
	dgb_printf_safe("[menu_dht_fun] ...\r\n");

	/* 识别到ENTER按键编码,则恢复DHT任务运行并获取温度/湿度 */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		dgb_printf_safe("[menu_dht_fun] FLAG_DHT_GET_DHT \r\n");
		
		/* 设置要显示的图片 */
		LCD_SAFE
		(					
		
			lcd_draw_picture(60,25,120,120,gImage_waether_180x210);
		);
		
		/* 恢复DHT任务运行 */ 
		vTaskResume(app_task_dht_handle);
	}

	/* 识别到BACK按键编码,则停止DHT任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* 挂起DHT任务 */
		vTaskSuspend(app_task_dht_handle);
	}	
}


/**
 * @brief 进入灯光设置菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_light_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_light_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */

		
	}

}


/**
 * @brief 进入电量显示菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_battery_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_battery_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */

		
	}

}


/**
 * @brief 进入修改时间日期菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_set_time_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_set_time_fun] ...\r\n");
	
	//清屏
	LCD_SAFE(
		/* 清屏 */
		lcd_clear(WHITE);
		lcd_fill(0,LCD_HEIGHT,LCD_WIDTH,80,WHITE);
		
	);

	/* 识别到ENTER按键编码,则恢复DHT任务运行并获取温度/湿度 */	
	if(menu_ext->key_fn == KEY_ENTER)
	{
			dgb_printf_safe("[menu_ble——time]\r\n");
			
			g_rtc_get_what = FLAG_RTC_GET_TIME;
			
			/* 设置要显示的图片与提示信息 */
			LCD_SAFE
			(					
				lcd_draw_picture(65, 50, 120, 120, gImage_pic_set_time_131x131);
			
				lcd_show_chn(10,180,37,BLACK,WHITE,32);		//请
				lcd_show_chn(40,180,38,BLACK,WHITE,32);		//连
				lcd_show_chn(72,180,39,BLACK,WHITE,32);		//接
				lcd_show_chn(104,180,40,BLACK,WHITE,32);	//蓝
				lcd_show_chn(136,180,59,BLACK,WHITE,32);	//牙
				lcd_show_chn(168,180,41,BLACK,WHITE,32);	//修
				lcd_show_chn(200,180,42,BLACK,WHITE,32);	//改
			);
		
		
		if( g_rtc_get_what!=FLAG_RTC_GET_TIME)
			return;		
		vTaskResume(app_task_ble_handle);
	}

	/* 识别到BACK按键编码,则停止BLE任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		vTaskSuspend(app_task_ble_handle);
		g_rtc_get_what=FLAG_RTC_GET_NONE;
	}
}


/**
 * @brief 进入记步显示菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_step_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_step_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_step_fun] FLAG_DHT_GET_Step\r\n");
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_STEP;
		
		
		/* 设置要显示的图片 */
		LCD_SAFE(
//			lcd_fill(0, 0, 240, 280,BLACK);
			lcd_draw_picture(60, 40, 120, 120, gImage_walk_count_166x100);
		);
		vTaskResume(app_task_mpu6050_handle);
	}
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		/* 挂起MPU6050任务 */
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
		vTaskSuspend(app_task_mpu6050_handle);
	}	
}


/**
 * @brief 进入久坐显示菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_sedentary_fun(void* pvParameters)
{
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_step_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		
		dgb_printf_safe("[menu_step_fun] FLAG_DHT_GET_Sedentary\r\n");
		
		g_mpu6050_get_what = FLAG_MPU6050_GET_STOP;
		
		
		/* 设置要显示的图片 */
		LCD_SAFE(
//			lcd_fill(0, 0, 240, 280,BLACK);
			lcd_draw_picture(60,70,120,120, gImage_walk_count_166x100);
		);
		vTaskResume(app_task_mpu6050_handle);
	}
	if(menu_ext->key_fn == KEY_BACK)
	{
		
		/* 挂起MPU6050任务 */
		g_mpu6050_get_what = FLAG_MPU6050_GET_NONE;
		vTaskSuspend(app_task_mpu6050_handle);
	}	
}


/**
 * @brief 进入心率血氧菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 识别到ENTER键,则根据传入的项目光标位置指定获取心率血氧,
 * 			 并恢复max30102任务的运行；
 * 			.识别到BACK键,则挂起max30102任务。	
 */
static void menu_pulse_fun(void* pvParameters)
{	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	dgb_printf_safe("[menu_pulse_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		dgb_printf_safe("[menu_pulse_fun] FLAG_DHT_GET_Pulse\r\n");
		
		/* 设置要显示的图片 */
		LCD_SAFE
		(
			lcd_show_chn(60,10,32,BLACK,WHITE,32);
			lcd_show_chn(105,10,33,BLACK,WHITE,32);
			lcd_show_chn(150,10,34,BLACK,WHITE,32);
		
			lcd_draw_picture(60,40,120,120,gImage_pic_in_the_pulse_120x120);
		);

		/* 恢复max30102任务运行 */ 
		vTaskResume(app_task_max30102_handle);
	}

	/* 识别到BACK按键编码,则停止max30102任务运行 */	
	if(menu_ext->key_fn == KEY_BACK)
	{
		/* 挂起max30102任务 */
		vTaskSuspend(app_task_max30102_handle);
	}	
}


/**
 * @brief 进入rfid菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无		
 */
static void menu_rfid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_rfid_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */
	
	
	
	}	
}
/**
 * @brief 进入camera菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_camera_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;

	

	dgb_printf_safe("[menu_camera_fun] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
	
	
	
	}	
}

/**
 * @brief 进入人脸识别菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_facid_fun(void* pvParameters)
{
	
	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;


	dgb_printf_safe("[menu_facid] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */


	}
}

/**
 * @brief 进入指纹识别菜单后执行相关的控制与配置
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 暂无	
 */
static void menu_touchid_fun(void* pvParameters)
{

	menu_ext_t *menu_ext = (menu_ext_t *)pvParameters;
	
	dgb_printf_safe("[menu_touchid] ...\r\n");

	/* 识别到ENTER按键编码*/	
	if(menu_ext->key_fn == KEY_ENTER)
	{
		/* 设置要显示的图片 */


	}
}

/**
 * @brief 获取当前菜单的项目总数
 * @param .menu 指向当前菜单
 * @retval 当前菜单项目总数
 * @details 无
 */
uint32_t menu_item_total(menu_t *menu)
{
	menu_t *m = menu;

	uint32_t item_count=0;

	while(m->item)
	{
		/* 指向下一个菜单 */
		m++;

		/* 统计项目数量 */
		item_count++;
	}
		
	return item_count;
}


/**
 * @brief 显示菜单项目
 * @param .menu_ext 指向当前菜单
 * @retval 当前菜单项目总数
 * @details 无
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
		/* 检查菜单项目标题有效性 */
		if(m->item == NULL || m->item[0]=='-' )
			break;		

		dgb_printf_safe("[menu_show] m->item is %s \r\n",m->item);

		LCD_SAFE
		(
			lcd_draw_picture(0,y,48,48,m->pic);
			lcd_show_string (80,y+6,m->item,BLACK,WHITE,32,0);	
		);
		
		
		/* 指向下一个菜单 */
		m++;

		/* y坐标下一行 */
		y+=60;
	}
	
	/* 拥有子菜单,显示光标 */
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
