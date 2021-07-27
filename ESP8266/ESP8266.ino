/************************************************
硬件：ALIENTEK精英STM32开发板、OpenMV、ATK-ESP8266、路由器、安卓/苹果手机等    
程序：《基于OpenMV的无人驾驶智能小车模拟系统》WiFi协议通讯程序
库版本：ST3.5.0
作者：谢青桦 @kelecn
日期：2021年03月20日
************************************************/
#define BLINKER_WIFI

#include <Blinker.h>
//#include <ArduinoJson.h>//使用JSON-v5版的库  使用<Blinker.h>库时不需要添加


char auth[] = "************";//点灯科技APP上给出的ID号

String comdata = " ";//串口接收数组
bool oState = false;//状态标志位
int  flag= 0;
int  l= 0;//短格式，减少延迟
int  a= 0;
int  d= 0;
int  z= 0;
BlinkerNumber Number0("num-wifi");//WIFI信号
BlinkerNumber Number1("num-led");//led信号
BlinkerNumber Number2("num-angle");//角度信号
BlinkerNumber Number3("num-distance");//距离信号
BlinkerButton Button0("btn-stoping");//停止状态按键
BlinkerButton Button1("btn-go");//前进状态按键
BlinkerButton Button2("btn-right");//右转状态按键
BlinkerButton Button3("btn-left");//左转状态按键
BlinkerButton Button4("btn-back");//后退状态按键
BlinkerButton Button5("btn-auto");//自动驾驶状态按键
//心跳包
void dataRead(const String & data)
{
    uint32_t BlinkerTime = millis();
}
//按下按键即会执行该函数
void button0_callback(const String & stoping) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 0;
}
//按下按键即会执行该函数
void button1_callback(const String & go) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 1;
}
//按下按键即会执行该函数
void button2_callback(const String & right) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 2;
}
//按下按键即会执行该函数
void button3_callback(const String & left) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 3;
}
//按下按键即会执行该函数
void button4_callback(const String & back) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 4;
}
//按下按键即会执行该函数
void button5_callback(const String & back) {
    if(oState == false)
      oState = true;
     else if(oState == true)
      oState = false;
      flag= 5;
}
/*初始化*/
void setup()
{
    // 初始化串口
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);   //初始化完成 灯灭
    if(!AutoConfig())//WIFI配网
    {
      SmartConfig();
    }
    // 初始化blinker
    Blinker.begin(auth,WiFi.SSID().c_str(),WiFi.psk().c_str());
    Button0.attach(button0_callback);//绑定按键的回调函数
    Button1.attach(button1_callback);//绑定按键的回调函数
    Button2.attach(button2_callback);//绑定按键的回调函数
    Button3.attach(button3_callback);//绑定按键的回调函数
    Button4.attach(button4_callback);//绑定按键的回调函数
    Button5.attach(button5_callback);//绑定按键的回调函数
}
/*主循环*/
void loop()
{
    Blinker.run();
    Number0.print(WiFi.RSSI());  //信号强度
    usartEvent();//串口中断
    l=int(z/10000);
    a=int((z-10000*l)/100);
    d=int(z-10000*l-100*a);
    if(oState == false && digitalRead(LED_BUILTIN)== LOW)//灯灭
    {
      digitalWrite(LED_BUILTIN,HIGH);//灯灭
      Serial.print(flag);
    }
     else if(oState == true && digitalRead(LED_BUILTIN)== HIGH)//灯亮
     {
       digitalWrite(LED_BUILTIN,LOW);//灯亮
       Serial.print(flag);
     }
     Number1.print(l);  //红绿灯信号强度
     Number2.print(a);  //角度信号强度
     Number3.print(d);  //距离信号强度
}
/*串口数据接收*/
void usartEvent(){
  comdata = "";
  while (Serial.available())//时刻读取硬件串口数据
  {
    comdata = Serial.readStringUntil('\n');//从串口缓存区读取字符到一个字符串型变量，直至读完或遇到某终止字符。
    UserData(comdata);//进行JOSN数据解析
  }
  while (Serial.read() >= 0){}//清除串口缓存
}
/*数据解析*/ 
void UserData(String content){
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc,content);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    return;
  }
  //oState =  doc["status"];
  //l =  doc["l"];
  //a =  doc["a"];
  //d =  doc["d"];
    z =  doc["z"];
  //Serial.printf("the status is %d",oState);
}
//自动配网设置
void SmartConfig()
{
  WiFi.mode(WIFI_STA);
  //Serial.println("\r\nWait for Smartconfig...");
  WiFi.beginSmartConfig();
  while (1)
  {
    //Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);//等待配网 闪烁
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);        
    delay(300);
    if (WiFi.smartConfigDone())
    {
      /*
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      */
      digitalWrite(LED_BUILTIN, HIGH);   //配网成功 LED常灭
      break;
    }
  }
}
//检测是否连接
bool AutoConfig()
{
    WiFi.begin();
    //如果觉得时间太长可改
    for (int i = 0; i < 10; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
          /*
          Serial.println("WIFI SmartConfig Success");
          Serial.printf("SSID:%s", WiFi.SSID().c_str());
          Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
          Serial.print("LocalIP:");
          Serial.print(WiFi.localIP());
          Serial.print(" ,GateIP:");
          Serial.println(WiFi.gatewayIP());
          */
          digitalWrite(LED_BUILTIN, HIGH);   //链接之前网络连接成功 LED常灭
          return true;
        }
        else
        {
          /*
          Serial.print("WIFI AutoConfig Waiting......");
          Serial.println(wstatus);
          */
          digitalWrite(LED_BUILTIN, HIGH);   //灭
          delay(1000);
          digitalWrite(LED_BUILTIN, LOW);   //亮 
          delay(1000);
        }
    }
    //Serial.println("WIFI AutoConfig Faild!" );
    return false;
}
