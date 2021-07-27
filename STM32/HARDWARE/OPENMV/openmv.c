/************************************************
硬件：ALIENTEK精英STM32开发板、OpenMV、ATK-ESP8266、HC-SR04等
连接：
--------------------------	
|  OpenMV   |   STM32    |
| :-------: | :--------: |
| P4(TXD)   | PA3(RXD)   |
| P5(RXD)   | PA2(TXD)   |
|    VCC    |    +5V     |
|    GND    |    GND     |
|    GND    |    GND     |
--------------------------
程序：OpenMV通讯程序
库版本：ST3.5.0
 
作者：谢青桦 @kelecn
日期：2021年03月25日
************************************************/

#include "sys.h"
#include "openmv.h"
#include "led.h"
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif 
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout2;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit2(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputcc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART2_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF2[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA2=0;       //接收状态标记	  
  
void usart2_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    //使能USART2,GPIOA时钟//USART2_TX   GPIOA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
   
    //USART2_RX      GPIOA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

    //USART2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器//USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;//串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    //收发模式

    USART_Init(USART2, &USART_InitStructure); //初始化串口2
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_Cmd(USART2, ENABLE);                    //使能串口2
}
  
extern int ov_frame;
    
u8 X,Y;    //定义数据 X->0无数据 1红灯 2绿灯;Y->理想情况50(中间)
   
void Data_Processing(u8 *data_buf,u8 num)
    
{
     
 int theta_org,rho_org;
    
    theta_org = (int)(*(data_buf+1)<<0) | (int)(*(data_buf+2)<<8) | (int)(*(data_buf+3)<<16) | (int)(*(data_buf+4)<<24) ;
     
    X = theta_org;
    
     
     
    rho_org = (int)(*(data_buf+5)<<0) | (int)(*(data_buf+6)<<8) | (int)(*(data_buf+7)<<16) | (int)(*(data_buf+8)<<24) ;
     
    Y = rho_org;
      
}
   
     
void Receive_Prepare(u8 data)
     
{
    
    static u8 RxBuffer[10];
    
    static u8  _data_cnt = 0;
    
    static u8 state = 0;
     
    if(state==0 && data==0xAA)
    
    {
          //LED0 =0;
			    //LED1 =1;
          state=1;
		      //delay_ms(1000); 
			   
     
    }
     
    else if(state==1 && data==0xAE)
     
    {
        //LED0 =1;
			  // LED1 =0;
        state=2;
			  // delay_ms(1000); 
      
        _data_cnt = 0;
     
    }
     
    else if(state==2)
      
    {
     
        RxBuffer[++_data_cnt]=data;
     
        if(_data_cnt>=8)
    
        { 
     
            state = 0;
     
            Data_Processing(RxBuffer,_data_cnt);
     
        }
      
    }
     
    else
     
        state = 0;
       
}
     
      
      
void USART2_IRQHandler(void)    
       
{
     
 u8 temp;
      
 if( USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET )
     
 {
     
  USART_ClearITPendingBit(USART2,USART_IT_RXNE);
     
  temp = USART_ReceiveData(USART2);
    
  Receive_Prepare(temp);
         
 }
       
}
#endif
