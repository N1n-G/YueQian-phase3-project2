#include "includes.h"

static	GPIO_InitTypeDef    GPIO_InitStructure;
static	USART_InitTypeDef  USART_InitStructure;
static	NVIC_InitTypeDef   NVIC_InitStructure;

extern QueueHandle_t g_queue_usart;
	
volatile uint8_t g_ble_buf[128]={0};
volatile uint8_t g_buf_size=0;


void blue_tooth_init(uint32_t baud)
{
	//�˿�BӲ��ʱ�Ӵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	//����3Ӳ��ʱ�Ӵ�
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	//����PB10 PB11ΪAFģʽ�����ù��ܣ�
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11; 	//9 10������
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;//���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//���٣��ٶ�Խ�ߣ���ӦԽ�죬���ǹ��Ļ����
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//��ʹ������������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//��������֧�ֺܶ๦�ܣ���Ҫָ�������ŵĹ��ܣ���ǰҪ�ƶ�֧��USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);	
	
	//����USART1��ز����������ʡ�����λ��ֹͣλ��У��λ
	USART_InitStructure.USART_BaudRate = baud;					//�����ʣ�����ͨ�ŵ��ٶ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//����ҪУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ�������ƹ��ܲ���Ҫ
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//���������ͺͽ�������
	USART_Init(USART3, &USART_InitStructure);
	
	//�����жϴ�����ʽ�����յ�һ���ֽڣ���֪ͨCPU����
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	//NVIC����������ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;			//�жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;	//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//��ͨ����NVIC����
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ��USART3����
	USART_Cmd(USART3, ENABLE);
	//PA6
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}


uint32_t ble_connect_sta_get(void)
{
	if(PAin(6))
	{
		return FLAG_BLE_STATUS_CONNECT;
	}
	else
	{
		return FLAG_BLE_STATUS_NONE;
	}
}


//����3�жϷ�����
void USART3_IRQHandler(void)
{
	uint8_t d;
	uint32_t ulReturn;

	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
	
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
	{
		d = USART_ReceiveData(USART3);
		g_ble_buf[g_buf_size++]=d;
		
		//�ȴ����ڷ������
		//while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
		if(d=='#' || g_buf_size>= sizeof(g_ble_buf))
		{
			
			//�����ж��¼�
			xQueueSendFromISR(g_queue_usart,(void *)&g_ble_buf,NULL);
			
			g_buf_size=0;
			memset((void *)g_ble_buf,0,64);
		}
		
		/* ����CPU���Ѿ���ɽ����ж����󣬿�����Ӧ�µĽ����ж����� */
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		
	}
	
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );

}
