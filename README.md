# 基于OpenMV的无人驾驶智能小车模拟系统

**作者：谢青桦@kelecn**

### 一、硬件模块

##### 1、STM32F103ZET6(正点原子精英板F103)

##### 2、OpenMV

##### 3、HC-SR04

##### 4、双路 H 桥电机驱动

##### 5、ESP8266

##### 6、直流电机、电源、模型车、导线等

### 二、编程软件

##### 1、OpenMV

使用OpenMV官方的[OpenMV IDE](https://singtown.com/openmv-download/)

##### 2、ESP8266

使用Arduino官方的[Arduino IDE](https://www.arduino.cc/en/software)

##### 3、STM32

使用[Keil uVision5](https://www2.keil.com/mdk5)（ARM版）

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
