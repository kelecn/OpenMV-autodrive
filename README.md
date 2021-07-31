# 基于OpenMV的无人驾驶智能小车模拟系统
![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post291.png)

**项目作者：@[kelecn](https://github.com/kelecn)**

If you cannot read Chinese, you can click here:[README-EN](/README-EN.html)

## 一、项目简介

基于机器视觉模块OpenMV采集车道、红绿灯、交通标志等模拟路况信息，实现一辆能车道保持、红绿灯识别、交通标志识别、安全避障以及远程WiFi控制的多功能无人驾驶小车。

**编程软件：**

| 硬件模块 | 编程软件                                                     |
| :------: | :----------------------------------------------------------- |
|  OpenMV  | 使用OpenMV官方的[OpenMV IDE](https://singtown.com/openmv-download/) |
| ESP8266  | 使用Arduino官方的[Arduino IDE](https://www.arduino.cc/en/software) |
|  STM32   | 使用ARM官方的[Keil uVision5](https://www2.keil.com/mdk5)（ARM版） |

**功能介绍：**

| 硬件模块 | 功能实现                                                     |
| :------: | :----------------------------------------------------------- |
|  OpenMV  | 主要是利用OpenMV进行路况信息（红绿灯、交通标志、车道）的采取，以及和STM32的通信，具体看OpenMV文件夹。 |
| ESP8266  | 主要是利用ESP8266与手机端进行远程的指令接收和数据交互，以及和STM32的通讯，具体看ESP8266文件夹。 |
|  STM32   | 主要是通过ESP8266接收远程控制指令和处理路况信息，并根据这些指令数据进行实时的PID控制小车运动。具体看STM32文件夹。 |

## 二、硬件系统

本项目《基于OpenMV的无人驾驶智能小车模拟系统》，主要依靠机器视觉模块OpenMV通过图像处理的方式获取实时的路况信息，以及超声波传感器获取障碍物距离信息，得到的路况数据再通过串口传输到主控器STM32上面，STM32会将实时的路况信息处理成智能小车的运动控制指令，让智能小车实现红绿灯识别、交通标志识别以及车道实时保持的功能，还有STM32也会通过WiFi模块ESP8266与手机端进行路况数据和控制指令的远程交互。硬件系统框图如下：

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post292.png)

下面简单介绍一下，整个系统用到的硬件模块。

|   硬件模块   | 型号                                                 |
| :----------: | ---------------------------------------------------- |
|    主控器    | STM32F103ZET6(正点原子精英板F103)                    |
|   视觉模块   | OpenMV4 H7 PLUS（STM32H750VBT6 +OV7725）             |
| 超声波传感器 | HC-SR04                                              |
|   WiFi模块   | ESP8266                                              |
|    执行器    | 双路 H 桥电机驱动、直流电机、开发板自带的LED和蜂鸣器 |
|   控制平台   | 安装Blinker APP的安卓/苹果手机                       |
|     其他     | 电源、模型车、导线若干等                             |

具体的硬件电路连接框图如下:

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post293.png)

## 三、软件系统

#### 1、OpenMV中的路况识别算法实现

本项目的主要路况数据信息都是基于OpenMV摄像头获取的图像进行图像处理得到的。要实现智能小车的自动驾驶行为，最起码要让小车识别到红绿灯、交通标志以及车道，后续主控器才能根据这些路况数据信息控制小车的运动。关于机器视觉模块OpenMV，之前我在《[初探机器视觉模块OpenMV](https://kelecn.top/posts/9237/)》里面已经介绍过了，这里不再赘述。

##### ①红绿灯识别

主要是对摄像头每帧拍摄到的图像进行图像进行阈值处理，再进行判断出现的究竟是哪种红绿灯（红灯、绿灯、黄灯），然后再将这个判定结果和其他两个数据一起打包通过串口发送出去。

**【程序流程图】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post294.png)

**【主要程序】**

```python
###################################开始####################################
...
sensor.set_pixformat(sensor.RGB565) # 图片格式设为 RGB565彩色图
light_threshold = [(59, 100, 26, 127, -128, 127),(59, 100, -128, -40, -128, 127)]; #设置红绿灯阈值，默认为0无红绿灯 1红灯 2绿灯 4黄灯
...
#定义寻找色块面积最大的函数
def find_max(blobs):    
    max_size=0
    for blob in blobs:
        if blob.pixels() > max_size:
            max_blob=blob
            max_size = blob.pixels()
return max_blob
#主循环
while(True):
    clock.tick() #追踪两个snapshots()之间经过的毫秒数
    img = sensor.snapshot() #拍一张照片，返回图像
    blobs = img.find_blobs(light_threshold,area_threshold=150); #找到红绿灯
    cx=0;cy=0;LED_color=0; #变量定义
    if blobs:
        max_b = find_max(blobs); #如果找到了目标颜色
        img.draw_rectangle(max_b[0:4]) #在Blob周围绘制一个矩形
        #用矩形标记出目标颜色区域
        img.draw_cross(max_b[5], max_b[6]) # cx, cy
        img.draw_cross(160, 120) # 在中心点画标记
        #在目标颜色区域的中心画十字形标记
        cx=max_b[5];
        cy=max_b[8];
        img.draw_line((160,120,cx,cy), color=(127));
        img.draw_string(cx, cy, "(%d, %d)"%(cx,cy), color=(127));
LED_color=cy; #红绿灯的阈值是数组里的cy（二进制）个
print(LED_color); #串行终端打印出 红绿灯序号数据
###################################结束####################################
```

##### ②交通标志识别

主要是利用NCC（Normalized Cross Correlation）归一化互相关算法来进行交通标志的图像识别与匹配。

**【NCC算法】**

NCC算法的基本实现原理：主要是通过求两幅大小相近的图像的相关系数矩阵来判别两幅图像是否相关。假设需要识别的初始图片$g$的大小为$m×n$，摄像头拍摄到的图片S的大小为$M×N$，其中的以$(x,y)$为左上角点与$g$大小相同的子图像为$S_{(x,y)}$​​​​​​​​。具体的利用NCC算法实现计算图像相似度的方法如下：

$\rho{(x,y)}$​​的定义为：随机变量$X$、$Y$的相关系数

$$
\rho{(x,y)}=\frac{\sigma(S_{x,y},g)}{\sqrt[]{D_{x,y}D}}
$$
  式中：$\sigma(S_{x,y},g)$​是$S_{x,y}$​和$g$​的协方差；

$D_{x,y}=\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(S_{x,y}(i,j)-\overline S_{x,y})^2}$​​为$S_{x,y}$​​的方差；

$D=\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(g(i,j)-\overline g)^2}$​​​为$g$的方差；

$\overline g$为$g$的灰度均值；

$\overline S_{x,y}$​​为$S_{x,y}$​​的灰度均值；

将$D_{x,y}$​和D代入$\rho{(x,y)}$​，会有：

$$
\rho{(x,y)=\frac{ \frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(S_{x,y}(i,j)-\overline S_{x,y})(g(i,j)-\overline g)}}{\sqrt[]{\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(S_{x,y}(i,j)-\overline S_{x,y})^2}}\sqrt[]{\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(g(i,j)-\overline g)^2}}}}
$$
其中，相关系数$\rho{(x,y)}$满足：$\rho{(x,y)\le1}$​。

$\rho{(x,y)}$越接近1，说明两幅图像越接近，也就是大图像的子集中越有可能包含有小图像。通过选取恰当的相关系数，我们就可以认为，相关系数大于该设定值的图像为所需识别的图像，也就是可以实现交通标识的识别。

**【程序流程图】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post295.png)

**【主要程序】**

```python
###################################开始####################################
...
sensor.set_pixformat(sensor.GRAYSCALE) #设置图片格式为灰度图
#导入图片模板
template1 = image.Image("/1.pgm") #直行
template2 = image.Image("/2.pgm") #向右转弯
template3 = image.Image("/3.pgm") #向左转弯
template4 = image.Image("/4.pgm") #停车让行
template5 = image.Image("/5.pgm") #鸣喇叭
#主循环
while (True):
    clock.tick()
    img = sensor.snapshot()
    flag=0
ratio=0
#匹配1.pgm（直行）串行终端打印go，flag=1
    r1 = img.find_template(template1, 0.70, step=4, search=SEARCH_EX)
    if r1:
        img.draw_rectangle(r1,color=(255,0,0))
        print("go")
        flag=1
        img.draw_string(10, 10, "%.d"%flag)
#匹配2.pgm（向右转弯）串行终端打印right，flag=2
    r2 = img.find_template(template2, 0.70, step=4, search=SEARCH_EX) 
    if r2:
        img.draw_rectangle(r2,color=(0,255,0))
        print("right")
        flag=2
        img.draw_string(10, 10, "%.d"%flag)
#匹配3.pgm（向左转弯）串行终端打印left，flag=3
    r3 = img.find_template(template3, 0.70, step=4, search=SEARCH_EX)
    if r3:
        img.draw_rectangle(r3,color=(255,0,0))
        print("left")
        flag=3
        img.draw_string(10, 10, "%.d"%flag)
#匹配4.pgm（停车让行）串行终端打印stop，flag=4
    r4 = img.find_template(template4, 0.70, step=4, search=SEARCH_EX) 
    if r4:
        img.draw_rectangle(r4,color=(255,255,0))
        print("stop")
        flag=4
        img.draw_string(10, 10, "%.d"%flag)
#匹配5.pgm（鸣喇叭）串行终端打印beep，flag=5
    r5 = img.find_template(template5, 0.70, step=4, search=SEARCH_EX)
    if r5:
        img.draw_rectangle(r5,color=(255,255,0))
        print("beep")
        flag=5
        img.draw_string(10, 10, "%.d"%flag)
###################################结束####################################
```

##### ③车道识别

 主要通过OpenMV模块，识别并跟踪车道阈值，通过几何运算出小车与车道中线的角度（偏左为正、偏右为负），反馈出小车与车道的真实偏离情况（可量化），后续用于PID控制。

**【程序流程图】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post296.png)

**【主要程序】**

```python
###################################开始####################################
...
sensor.set_pixformat(sensor.RGB565) # 图片格式设为 RGB565彩色图
road_threshold = [(23, 0, -45, 19, -31, 28)]; #黑线道路
ROI = (0, 100, 320, 40)
...
#省略了识别车道边框函数
#偏移角度计算算法
def get_direction(left_blob, right_blob):
    # 根据图像中的三块左中右的白色部分，计算出摄像头偏转角度
    # ratio < 0 左拐，小车在车道偏右位置
    # ratio > 0 右拐，小车在车道偏左位置

    MAX_WIDTH = 320
    # 调节theta来设置中间宽度的比重， theta越高ratio越靠近0
    # 需要根据赛道宽度与摄像头高度重新设定到合适大小
    theta = 0.01
    # 这里的b是为了防止除数是0的情况发生， 设定一个小一点的值
    b = 3
    x1 = left_blob.x() - int(0.5 * left_blob.w()) #左边黑线中心x值
    x2 = right_blob.x() + int(0.5 * right_blob.w()) #右边黑线中心x值
#车道信息计算
    w_left = x1 #左边车道外宽度
    w_center = math.fabs(x2 - x1) #车道中心x值
    w_right = math.fabs(MAX_WIDTH - x2) #右边车道外宽度
#计算摄像头偏移角度
    direct_ratio = (w_left + b + theta * w_center) / (w_left + w_right + 2 * b + 2 * theta * w_center) - 0.5
#返回摄像头偏移角度
return direct_ratio
#省略了可视化绘图函数
...
while(True): #主循环
clock.tick() #追踪两个snapshots()之间经过的毫秒数
    img = sensor.snapshot() #拍一张照片，返回图像
    blobs = img.find_blobs(road_threshold, roi=ROI, merge=True);
    a=0;ratio=0;
    if blobs:
        left_blob, right_blob = get_top2_blobs(blobs)

        if(left_blob == None or right_blob == None):
            print("Out Of Range")
            continue
        else:
#画出车道左边线
            img.draw_rectangle(left_blob.rect())
            img.draw_cross(left_blob.cx(), left_blob.cy())
#画出车道右边线
            img.draw_rectangle(right_blob.rect())
            img.draw_cross(right_blob.cx(), right_blob.cy())
#可视化显示偏转角度
            direct_ratio = get_direction(left_blob, right_blob)
            draw_direct(img,direct_ratio)
            ratio=int(math.degrees(direct_ratio)) #偏转角度转成弧度值
            img.draw_string(10, 10, "%.d"%ratio) #帧缓冲区实时画出偏转角度
            print(ratio) #串行终端打印偏转角度
    img.draw_rectangle(ROI,color=(255, 0, 0)) #画出感兴趣区域
###################################结束####################################
```

#### 2、基于ESP8266的远程控制平台实现

主要是利用点灯科技-[Blinker](https://diandeng.tech/home)物联网平台搭建控制APP的UI界面，以及调用Blinker的控制代码，实现智能小车控制指令的下发与路况数据的上传。

**【远程控制平台UI界面】**

<img src="https://cdn.jsdelivr.net/gh/kelecn/images@master/post297.png" style="zoom: 25%;" />

**【UI配置代码】**

直接使用 [点灯.blinker](https://www.diandeng.tech/home) APP导入配置代码即可获得和我一样的UI布局。

```
{¨config¨{¨headerColor¨¨transparent¨¨headerStyle¨¨dark¨¨background¨{¨img¨¨assets/img/headerbg.jpg¨¨isFull¨«}}¨dashboard¨|{¨type¨¨btn¨¨ico¨¨fad fa-arrow-alt-up¨¨mode¨É¨t0¨¨前进¨¨t1¨¨文本2¨¨bg¨Ì¨cols¨Ë¨rows¨Ë¨key¨¨btn-go¨´x´Ì´y´Ï¨speech¨|÷¨clr¨¨#076EEF¨}{ßAßBßC¨fad fa-arrow-alt-down¨ßEÉßF¨后退¨ßHßIßJÌßKËßLËßM¨btn-back¨´x´Ì´y´¤CßO|÷ßPßQ¨lstyle¨É}{ßAßBßC¨fad fa-arrow-alt-right¨ßEÉßF¨右转¨ßHßIßJÌßKËßLËßM¨btn-right¨´x´Ï´y´ÒßO|÷ßPßQßUÉ}{ßAßBßC¨fad fa-arrow-alt-left¨ßEÉßF¨左转¨ßHßIßJÌßKËßLËßM¨btn-left¨´x´É´y´ÒßO|÷ßPßQßUÉ}{ßAßBßC¨fad fa-power-off¨ßEÉßF¨停车¨ßHßIßJÌßKËßLËßM¨btn-stoping¨´x´Ï´y´ÏßO|÷ßPßQßUÉ}{ßA¨tex¨ßF¨😋小车远程监控系统😋¨ßH´´ßJËßC´´ßKÍßLÊßM´´´x´Ë´y´ËßO|÷ßPßQßUÊ}{ßA¨num¨ßF¨障碍物距离¨ßC¨fad fa-route¨ßPßQ¨min¨É¨max¨¢1c¨uni¨¨cm¨ßJÉßKÍßLËßM¨num-distance¨´x´É´y´¤EßO|÷ßUÊ}{ßAßgßF¨小车偏移角度¨ßC¨fad fa-tachometer-alt-fast¨ßPßQßjÉßkº0ßl´º´ßJÉßKÍßLËßM¨num-angle¨´x´Í´y´¤EßO|÷ßUÊ}{ßAßgßF¨红绿灯(红1绿2)¨ßC¨fad fa-siren-on¨ßPßQßjÉßkËßl´´ßJÉßKËßLËßM¨num-led¨´x´É´y´ÏßO|÷ßUÉ}{ßA¨deb¨ßEÉßJÉßKÑßLÌßM¨debug¨´x´É´y´ÌßO|÷ßUÉ}{ßAßgßF¨WIFI信号¨ßC¨fad fa-signal-4¨ßP¨#389BEE¨ßjÉßkº0ßl¨dbm¨ßJÉßKËßLËßM¨num-wifi¨´x´Ï´y´ÉßO|÷ßUÉ}{ßAßBßC¨fad fa-repeat-alt¨ßEÊßF¨自动驾驶模式¨ßHßIßJËßKËßLËßM¨btn-auto¨´x´Ì´y´ÒßO|÷ßPßQßUÉ}÷¨actions¨|¦¨cmd¨¦¨switch¨‡¨text¨‡´on´¨打开?name¨¨off¨¨关闭?name¨—÷¨triggers¨|{¨source¨ß16¨source_zh¨¨开关状态¨¨state¨|´on´ß19÷¨state_zh¨|´打开´´关闭´÷}÷}
```

**【控制指令与监控数据】**

| 名称         | 功能按钮/数据接收框的功能       | 数据键名     | 指令 |
| ------------ | ------------------------------- | ------------ | ---- |
| WiFi信号     | 接收WiFi信号数据                | num-wifi     | —    |
| 红绿灯数据   | 接收红绿灯数据（无0，红1，绿2） | num-led      | —    |
| 小车偏移角度 | 接收小车偏移角度                | num-angle    | —    |
| 障碍物距离   | 接收障碍物距离数据              | num-distance | —    |
| 停车         | 发出停车指令                    | btn-stoping  | 0    |
| 前进         | 发出前进指令                    | btn-go       | 1    |
| 右转         | 发出右转指令                    | btn-right    | 2    |
| 左转         | 发出左转指令                    | btn-left     | 3    |
| 后退         | 发出后退指令                    | btn-back     | 4    |
| 自动驾驶     | 发出自动驾驶指令                | btn-auto     | 5    |
| Debug        | 显示收发数据的原始数据格式      | —            | —    |

**【程序流程图】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post298.png)

**【主要程序】**

```c
/***********************************开始***********************************/
...
int  flag= 0;                        //按键标志位
int  l= 0;                          //红绿灯数据
int  a= 0;                         //角度数据
int  d= 0;                        //距离数据
int  z= 0;                        //json解析出来的数据
BlinkerNumber Number0("num-wifi");//WIFI信号
BlinkerNumber Number1("num-led");//红绿灯信号
BlinkerNumber Number2("num-angle");//角度信号
BlinkerNumber Number3("num-distance");//距离信号
BlinkerButton Button0("btn-stoping");//停止状态按键
BlinkerButton Button1("btn-go");//前进状态按键
BlinkerButton Button2("btn-right");//右转状态按键
BlinkerButton Button3("btn-left");//左转状态按键
BlinkerButton Button4("btn-back");//后退状态按键
BlinkerButton Button5("btn-auto");//自动驾驶状态按键
...
/*主循环*/
void loop()
{
    Blinker.run();
    Number0.print(WiFi.RSSI());  //发送信号强度
    usartEvent();//串口中断
    l=int(z/10000);          //解析红绿灯数据
    a=int((z-10000*l)/100);  //解析偏移角度数据
d=int(z-10000*l-100*a); //解析距离数据
    Number1.print(l);  //发送红绿灯信号
    Number2.print(a);  //发送角度信号
    Number3.print(d);  //发送距离信号
//发送控制指令，灯的亮灭，主要是检查WiFi模块是否接收到数据
    if(oState == false && digitalRead(LED_BUILTIN)== LOW)//灯灭
    {
      digitalWrite(LED_BUILTIN,HIGH);//灯灭
      Serial.print(flag);                //发送指令
    }
     else if(oState == true && digitalRead(LED_BUILTIN)== HIGH)//灯亮
     {
       digitalWrite(LED_BUILTIN,LOW);//灯亮
       Serial.print(flag);                 //发送指令
     }
}
//Blinker初始化略
//WiFi连接信号检测略
//STM32数据上传解析略
...
/***********************************结束***********************************/
```

#### 3、智能小车的无人控制方案实现

智能小车在接收到ESP8266的控制指令和OpenMV路况数据，会根据这些指令数据进行小车运动的控制。

**【程序流程图】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post299.png)

**【PID控制算法】**

关于直流电机的PID调节，主要用来实现车道保持功能。通过OpenMV返回的偏转角度，进行实时调节电机PWM输出，使得偏转角度$Y=50$​（也就是小车与中线的偏转角为0，由于之前为了传输方便整体加上了50）。故将设定值定为50，通过实时返回的$Y$值与50做差值运算，得到PID的输入偏差，通过位置式PID返回实时的PWM值。关于[PID控制算法](https://kelecn.top/posts/16358/)，之前也有介绍到，这里不再深入赘述。
$$
PWM=K_P\theta(t)+K_i\sum_{t=0}\theta(t)+K_d[\theta(t)-\theta(t-1)]
$$
其中为$\theta(t)$​​是本次OpenMV返回的偏移角度数据$Y$​​与50的差值，$\theta(t-1)$​​为上一个$Y$​​​与50的差值。

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post299-1.png)

 **【模拟环境】**

<img src="https://cdn.jsdelivr.net/gh/kelecn/images@master/post299-2.png" style="zoom: 33%;" />

**【主要程序】**

略，详看：[OpenMV-autodrive](https://github.com/kelecn/OpenMV-autodrive)

