#include "includes.h"

static EXTI_InitTypeDef	EXTI_InitStructure;
static NVIC_InitTypeDef	NVIC_InitStructure;

static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_DateTypeDef  RTC_DateStructure;
static RTC_InitTypeDef  RTC_InitStructure;
static RTC_AlarmTypeDef RTC_AlarmStructure;

void rtc_init(void)
{
	/* Enable the PWR clock，使能电源管理时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	/* Allow access to RTC，允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSE
	//打开LSE振荡时钟
	RCC_LSEConfig(RCC_LSE_ON);

	//检查LSE振荡时钟是否就绪
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//为RTC选择LSE作为时钟源
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
#endif

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSI	
	RCC_LSICmd(ENABLE);
	
	/* Wait till LSI is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}
	
	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif
	
	/* Enable the RTC Clock，使能RTC的硬件时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation，等待所有RTC相关寄存器就绪 */
	RTC_WaitForSynchro();
	
#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSE
	/* Configure the RTC data register and RTC prescaler，配置RTC数据寄存器与RTC的分频值 */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//异步分频系数
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;				//同步分频系数
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
	RTC_Init(&RTC_InitStructure);
#endif

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSI	
	/* Configure the RTC data register and RTC prescaler，配置RTC数据寄存器与RTC的分频值 */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//异步分频系数
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//同步分频系数
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
	RTC_Init(&RTC_InitStructure);

#endif
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x8888)
	{
		//配置日期
		RTC_DateStructure.RTC_Year = 0x24;						// 2024年
		RTC_DateStructure.RTC_Month = RTC_Month_October;		// 10月
		RTC_DateStructure.RTC_Date = 0x30;						// 30号
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Wednesday;	// 星期三
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);


		//配置时间 17:14:50
		RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
		RTC_TimeStructure.RTC_Hours = 0x17;
		RTC_TimeStructure.RTC_Minutes = 0x14;
		RTC_TimeStructure.RTC_Seconds = 0x00; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
		
		// 向备份寄存器0写入0x8888
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x8888);
	}
	
	RTC_WakeUpCmd(DISABLE);
	
	//唤醒时钟源的硬件时钟频率为1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//只进行一次计数
	RTC_SetWakeUpCounter(0);
	
	RTC_WakeUpCmd(ENABLE);

	//配置中断的触发方式：唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	
	RTC_ClearFlag(RTC_FLAG_WUTF);
	
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//RTC手册规定
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//优先级
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void rtc_alarm_init(void)
{
	/* Enable RTC Alarm A Interrupt，允许RTC的A闹钟触发中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* 清空标志位 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);

	/*使能外部中断控制线17的中断*/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/*使能闹钟的中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
}

void rtc_alarm_set(RTC_AlarmTypeDef rtc_alarm_structure)
{
	/* 关闭闹钟，若不关闭，配置闹钟触发的中断有BUG，无论怎么配置，只要到00秒，则触发中断*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* Configure the RTC Alarm A register，配置RTC的A闹钟 */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &rtc_alarm_structure);
	
	/* Enable the alarm ，让RTC的闹钟A工作*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}


/* second范围:1~59 */
void rtc_alarm_set_in_second(uint32_t second)
{

	/* 关闭闹钟，若不关闭，配置闹钟触发的中断有BUG，无论怎么配置，只要到00秒，则触发中断*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);  
	
	
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12   = RTC_TimeStructure.RTC_H12;

	if(RTC_TimeStructure.RTC_Seconds+second>=59)
	{
		RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = RTC_TimeStructure.RTC_Seconds+second-59;
		RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = RTC_TimeStructure.RTC_Minutes+1;

	}
	else
	{
		RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = RTC_TimeStructure.RTC_Seconds+second;
		RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = RTC_TimeStructure.RTC_Minutes;
	}

	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = RTC_TimeStructure.RTC_Hours;
	
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

	/* Configure the RTC Alarm A register，配置RTC的A闹钟 */
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* Enable the alarm ，让RTC的闹钟A工作*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}

void  RTC_WKUP_IRQHandler(void)
{
	uint32_t ulReturn;

	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	//检测标志位
	if(RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		
		//设置事件标志组
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_RTC_WAKEUP,&xHigherPriorityTaskWoken);	
		
		//清空标志位	
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		EXTI_ClearITPendingBit(EXTI_Line22);
	}

	/* 若接收消息队列任务的优先级高于当前运行的任务，则退出中断后立即进行任务切换，执行前者;
	   否则等待下一个时钟节拍才进行任务切换
	*/
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}	
	
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );			
}

