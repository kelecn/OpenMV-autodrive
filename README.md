# 基于OpenMV的无人驾驶智能小车模拟系统
![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post291.png)
**项目作者：@[kelecn](https://github.com/kelecn)**

### 一、项目简介

基于机器视觉模块OpenMV采集车道、红绿灯、交通标志等模拟路况信息，实现一辆能车道保持、红绿灯识别、交通标志识别、安全避障以及远程WiFi控制的多功能无人驾驶小车。

**编程软件：**

| 硬件模块 | 编程软件                                                     |
| :------: | :----------------------------------------------------------- |
|  OpenMV  | 使用OpenMV官方的[OpenMV IDE](https://singtown.com/openmv-download/) |
| ESP8266  | 使用Arduino官方的[Arduino IDE](https://www.arduino.cc/en/software) |
|  STM32   | 使用ARM官方的[Keil uVision5](https://www2.keil.com/mdk5)（ARM版） |

### 二、硬件系统

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

### 三、软件系统

##### 1、OpenMV

主要是利用OpenMV进行路况信息（红绿灯、交通标志、车道）的采取。具体看OpenMV文件夹。

##### 2、ESP8266

主要是利用ESP8266与手机端进行远程通信。具体看ESP8266文件夹。

##### 3、STM32

主要是接受远程控制指令和处理路况信息，并根据这些指令数据进行实时的PID控制小车运动。具体看STM32文件夹。

##### 4、Blinker APP端配置代码

直接使用 [点灯.blinker](https://www.diandeng.tech/home) APP导入配置代码即可获得一样的UI布局。

```
{¨config¨{¨headerColor¨¨transparent¨¨headerStyle¨¨dark¨¨background¨{¨img¨¨assets/img/headerbg.jpg¨¨isFull¨«}}¨dashboard¨|{¨type¨¨btn¨¨ico¨¨fad fa-arrow-alt-up¨¨mode¨É¨t0¨¨前进¨¨t1¨¨文本2¨¨bg¨Ì¨cols¨Ë¨rows¨Ë¨key¨¨btn-go¨´x´Ì´y´Ï¨speech¨|÷¨clr¨¨#076EEF¨}{ßAßBßC¨fad fa-arrow-alt-down¨ßEÉßF¨后退¨ßHßIßJÌßKËßLËßM¨btn-back¨´x´Ì´y´¤CßO|÷ßPßQ¨lstyle¨É}{ßAßBßC¨fad fa-arrow-alt-right¨ßEÉßF¨右转¨ßHßIßJÌßKËßLËßM¨btn-right¨´x´Ï´y´ÒßO|÷ßPßQßUÉ}{ßAßBßC¨fad fa-arrow-alt-left¨ßEÉßF¨左转¨ßHßIßJÌßKËßLËßM¨btn-left¨´x´É´y´ÒßO|÷ßPßQßUÉ}{ßAßBßC¨fad fa-power-off¨ßEÉßF¨停车¨ßHßIßJÌßKËßLËßM¨btn-stoping¨´x´Ï´y´ÏßO|÷ßPßQßUÉ}{ßA¨tex¨ßF¨😋小车远程监控系统😋¨ßH´´ßJËßC´´ßKÍßLÊßM´´´x´Ë´y´ËßO|÷ßPßQßUÊ}{ßA¨num¨ßF¨障碍物距离¨ßC¨fad fa-route¨ßPßQ¨min¨É¨max¨¢1c¨uni¨¨cm¨ßJÉßKÍßLËßM¨num-distance¨´x´É´y´¤EßO|÷ßUÊ}{ßAßgßF¨小车偏移角度¨ßC¨fad fa-tachometer-alt-fast¨ßPßQßjÉßkº0ßl´º´ßJÉßKÍßLËßM¨num-angle¨´x´Í´y´¤EßO|÷ßUÊ}{ßAßgßF¨红绿灯(红1绿2)¨ßC¨fad fa-siren-on¨ßPßQßjÉßkËßl´´ßJÉßKËßLËßM¨num-led¨´x´É´y´ÏßO|÷ßUÉ}{ßA¨deb¨ßEÉßJÉßKÑßLÌßM¨debug¨´x´É´y´ÌßO|÷ßUÉ}{ßAßgßF¨WIFI信号¨ßC¨fad fa-signal-4¨ßP¨#389BEE¨ßjÉßkº0ßl¨dbm¨ßJÉßKËßLËßM¨num-wifi¨´x´Ï´y´ÉßO|÷ßUÉ}{ßAßBßC¨fad fa-repeat-alt¨ßEÊßF¨自动驾驶模式¨ßHßIßJËßKËßLËßM¨btn-auto¨´x´Ì´y´ÒßO|÷ßPßQßUÉ}÷¨actions¨|¦¨cmd¨¦¨switch¨‡¨text¨‡´on´¨打开?name¨¨off¨¨关闭?name¨—÷¨triggers¨|{¨source¨ß16¨source_zh¨¨开关状态¨¨state¨|´on´ß19÷¨state_zh¨|´打开´´关闭´÷}÷}
```
