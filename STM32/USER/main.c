/************************************************
硬件：ALIENTEK精英STM32开发板、OpenMV、ATK-ESP8266、HC-SR04等		
程序：《基于OpenMV的无人驾驶智能小车模拟系统》主控程序
库版本：ST3.5.0
作者：谢青桦 @kelecn
日期：2021年03月25日
************************************************/

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "beep.h"
#include "timer.h"
#include "wave.h"
#include "sys.h"
#include "pwm.h"
#include "usart.h"
#include "openmv.h"
#include "esp8266.h"
#include "stdio.h"
#include <stdlib.h>

u16 rlen=0,aaa=0;                                 //定义WIFI接收数组长度
u8 aw[10]={48,49,50,51,52,53,54,55,56,57};        //ASCII码0123456789
u8 flag=0;                                        //变量储存WIFI指令
int Position_PID(int a, int b);                   //预定义PID控制器 
void autodrive (int a, int b,int PID);            //预定义自动驾驶模式        

 int main(void)
 {		
	 int z =0;                                      //封装数据X、Y、Distance
	 int d=0;                                       //Distance 取整
   int a,b=0;                                     //PWM控制初始化值
	 float PID=0;                                   //PID控制输出的PWM值
	 
	delay_init();	    	 														//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	  												  	//串口1初始化为9600
 	LED_Init();		  		 														//初始化与LED连接的硬件接口
	BEEP_Init();         	 													//初始化蜂鸣器端口
	usart2_init(115200);  												  //OpenMV初始化为115200
	usart3_init(115200);  												  //ESP8266初始化为115200
	Timer_SRD_Init(5000,7199);											//用于超声波的定时器4初始化
	Wave_SRD_Init();                                //HC-SR04超声波模块初始化
	while(1)
	{	 		
		Wave_SRD_Strat();                             //超声波激发信号
		d=Distance;                                   //Distance 取整
		z= 10000*0+100*0+d;                           //路况数据打包
		a=880;b=870;                                  //基础速度 左右电机转速不一样，最大1000
		u3_printf("{\"z\":%d}",z);                    //发送路况数据
		PID=Position_PID(50,Y);                       //PID控制器
		BEEP=0;
		LED0=1;
		switch(flag){
			case 0:Go_stop ();break;						//停车
			case 1:Go_forward(a,b);break;				//前进
			case 2:Go_right(a-100,b-100);break; //右转
		  case 3:Go_left(a-100,b-100);break;  //左转
			case 4:Go_back(a,b);break;          //后退
			case 5:autodrive(a,b,PID);break;    //自动
		}
//WIFI接收数据函数
		 if(USART3_RX_STA&(1<<15))	//接收到数据
			                          //[15]:0,没有接收到数据;1,接收到了一批数据.
                                //[14:0]:接收到的数据长度
	   {	
      rlen=USART3_RX_STA&0X7FFF;//得到本次接收到的数据长度
			USART3_RX_BUF[rlen]=0;		//添加结束符 
			if(USART3_RX_BUF[rlen-1]==aw[0])     //接收0  停止 ASCII码48 
			{
				flag=0;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[1])//接收1 前进
			{
				flag=1;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[2])//接收2 右转
			{
				flag=2;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[3])//接收3 左转
			{
				flag=3;
			}
      else if(USART3_RX_BUF[rlen-1]==aw[4])//接收4 后退
			{
				flag=4;
			}
      else if(USART3_RX_BUF[rlen-1]==aw[5])//接收5 自动模式 
     {
        flag=5;
      }
	    //清除标志位
			delay_ms(100);		
			USART3_RX_STA=0;
	   }
	}
}
//自动驾驶模式 红绿灯以及车道同时识别 PID  由于成本有限,没有做多摄像头方案,交通标志识别功能单独测试即可.
void autodrive (int a, int b, int PID)
	{
    if(Distance<20)          //简单避障
	 {
		      BEEP=1;
					Go_stop();
		}else if (Distance>20)
		{
			    BEEP=0;
		if(X==1)                //红灯停车
		{
			    LED0=0;
					Go_stop();
		}else if (X==0||X==2)   //无红绿灯或者绿灯继续行驶 车道保持
		{
			   LED0=1;
		 if(Y>50)               //小车偏左  
		 {
		  Go_forward(PID,b);
		 }else if(Y<50)         //小车偏右  
			{
			Go_forward(a,PID);
			}
		}
	}
}

//位置式PID控制器
int Position_PID (int Encoder,int Target) //目标值50  输入值Y，即角度
  { 	
	 float Position_KP=160,Position_KI=0.01,Position_KD=90;
   static float Bias,Pwm,Integral_bias,Last_Bias;
   Bias=Encoder-Target; //计算偏差
   Integral_bias+=Bias; //求出偏差的积分	     
   Pwm=Position_KP*Bias+Position_KI*Integral_bias+Position_KD*(Bias-Last_Bias);       
   Last_Bias=Bias;       //保存上一次偏差 
   return Pwm;           //输出
  }

