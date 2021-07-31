# OpenMV-based driverless intelligent car simulation system
![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post291.png)

**Project author: @[kelecn](https://github.com/kelecn)**

## 1. Project Introduction

Based on the machine vision module OpenMV, it collects simulated road condition information such as lanes, traffic lights, and traffic signs to realize a multifunctional driverless car capable of lane keeping, traffic light recognition, traffic sign recognition, safe obstacle avoidance, and remote WiFi control.

**Programming software:**

| Hardware Module | Programming Software |
| :------: | :--------------------------------------- -------------------- |
| OpenMV | Use OpenMV official [OpenMV IDE](https://singtown.com/openmv-download/) |
| ESP8266 | Use the official Arduino [Arduino IDE](https://www.arduino.cc/en/software) |
| STM32 | Use ARM official [Keil uVision5](https://www2.keil.com/mdk5) (ARM version) |

**Features:**

| Hardware module | Function realization |
| :------: | :--------------------------------------- -------------------- |
| OpenMV | Mainly use OpenMV to take traffic information (traffic lights, traffic signs, lanes) and communicate with STM32, see the OpenMV folder for details. |
| ESP8266 | The main purpose is to use ESP8266 to remotely receive commands and interact with the mobile phone, as well as to communicate with STM32. See the ESP8266 folder for details. |
| STM32 | It is mainly to receive remote control instructions and process road condition information through ESP8266, and carry out real-time PID control of the car movement according to these instruction data. See the STM32 folder for details. |

## 2.hardware system

This project, "OpenMV-based driverless intelligent car simulation system", mainly relies on the machine vision module OpenMV to obtain real-time road condition information through image processing, and ultrasonic sensors to obtain obstacle distance information, and the obtained road condition data is transmitted through the serial port. On the main controller STM32, the STM32 will process the real-time road condition information into the motion control instructions of the smart car, allowing the smart car to realize the functions of traffic light recognition, traffic sign recognition and real-time lane maintenance. The STM32 will also communicate with the mobile phone through the WiFi module ESP8266 The terminal carries out the remote interaction of road condition data and control commands. The hardware system block diagram is as follows:

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post292.png)

The following briefly introduces the hardware modules used in the entire system.

| Hardware Module | Model |
| :----------: | ------------------------------------ ---------------- |
| Main Controller | STM32F103ZET6 (Punctual Atom Elite Board F103) |
| Vision Module | OpenMV4 H7 PLUS (STM32H750VBT6 +OV7725) |
| Ultrasonic Sensor | HC-SR04 |
| WiFi Module | ESP8266 |
| Actuator | Dual H-bridge motor drive, DC motor, LED and buzzer built in the development board |
| Control platform | Android/iPhone with Blinker APP installed |
| Others | Power supply, model car, wires, etc. |

The specific hardware circuit connection block diagram is as follows:

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post293.png)

## 3. software system

#### 1. Implementation of road condition recognition algorithm in OpenMV

The main road condition data information of this project is obtained by image processing based on the image obtained by the OpenMV camera. To realize the autonomous driving behavior of the smart car, at least the car must recognize the traffic lights, traffic signs, and lanes, so that the subsequent main controller can control the movement of the car based on these road conditions. Regarding the machine vision module OpenMV, I have already introduced it in the "[First Exploration of Machine Vision Module OpenMV](https://kelecn.top/posts/9237/)", so I won’t repeat it here.

##### ①Traffic light recognition

The main purpose is to perform threshold processing on the image captured by the camera for each frame, and then determine which traffic light (red light, green light, yellow light) appears, and then package this determination result and the other two data together. The serial port is sent out.

**【Program flow chart】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post294.png)

**【Main Program】**

```python
###################################Start############## #####################
...
sensor.set_pixformat(sensor.RGB565) # The picture format is set to RGB565 color map
light_threshold = [(59, 100, 26, 127, -128, 127),(59, 100, -128, -40, -128, 127)]; #Set the traffic light threshold, the default is 0, no traffic light 1 red light 2 Green light 4 yellow light
...
#Define the function to find the largest area of ​​the color patch
def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob.pixels()> max_size:
            max_blob=blob
            max_size = blob.pixels()
return max_blob
#Main loop
while(True):
    clock.tick() #Track the number of milliseconds that have passed between two snapshots()
    img = sensor.snapshot() #take a photo and return the image
    blobs = img.find_blobs(light_threshold,area_threshold=150); #Find traffic lights
    cx=0;cy=0;LED_color=0; #variable definition
    if blobs:
        max_b = find_max(blobs); #If the target color is found
        img.draw_rectangle(max_b[0:4]) #Draw a rectangle around the Blob
        #Mark the target color area with a rectangle
        img.draw_cross(max_b[5], max_b[6]) # cx, cy
        img.draw_cross(160, 120) # Draw a mark at the center point
        #Draw a cross mark in the center of the target color area
        cx=max_b[5];
        cy=max_b[8];
        img.draw_line((160,120,cx,cy), color=(127));
        img.draw_string(cx, cy, "(%d, %d)"%(cx,cy), color=(127));
LED_color=cy; #The threshold of traffic lights is cy (binary) in the array
print(LED_color); #Serial terminal prints traffic light serial number data
###################################end############## #####################
```

##### ②Traffic sign recognition

It mainly uses NCC (Normalized Cross Correlation) normalized cross-correlation algorithm for image recognition and matching of traffic signs.

**【NCC Algorithm】**

The basic implementation principle of the NCC algorithm: It is mainly to determine whether two images are related by finding the correlation coefficient matrix of two images of similar size. Suppose the size of the initial picture $g$ to be recognized is $m×n$, the size of the picture S taken by the camera is $M×N$, where $(x,y)$ is the upper left corner point and $g The sub-images with the same size are $S_{(x,y)}$​​​​​​​​. The specific method of using the NCC algorithm to calculate the image similarity is as follows:

The definition of $\rho{(x,y)}$​​ is: the correlation coefficient of the random variables $X$ and $Y$

$$
\rho{(x,y)}=\frac{\sigma(S_{x,y},g)}{\sqrt[]{D_{x,y}D}}
$$
  Where: $\sigma(S_{x,y},g)$​ is the covariance of $S_{x,y}$​ and $g$​;

$D_{x,y}=\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(S_{x,y}(i,j )-\overline S_{x,y})^2}$​​is the variance of $S_{x,y}$​​;

$D=\frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(g(i,j)-\overline g)^2}$ ​​​Is the variance of $g$;

$\overline g$ is the mean gray value of $g$;

$\overline S_{x,y}$​​is the mean gray value of $S_{x,y}$​​;

Substituting $D_{x,y}$​ and D into $\rho{(x,y)}$​, there will be:

$$
\rho{(x,y)=\frac{ \frac {1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(S_{x,y} (i,j)-\overline S_{x,y})(g(i,j)-\overline g)}}{\sqrt[]{\frac {1}{mn}\sum_{i=1} ^{m} \sum_{j=1}^{n}{(S_{x,y}(i,j)-\overline S_{x,y})^2}}\sqrt[]{\frac { 1}{mn}\sum_{i=1}^{m} \sum_{j=1}^{n}{(g(i,j)-\overline g)^2}}}}
$$
Among them, the correlation coefficient $\rho{(x,y)}$ satisfies: $\rho{(x,y)\le1}$​.

The closer $\rho{(x,y)}$ is to 1, the closer the two images are, which means that the subset of large images is more likely to contain small images. By selecting the appropriate correlation coefficient, we can consider that the image with the correlation coefficient greater than the set value is the image that needs to be recognized, that is, the recognition of the traffic sign can be realized.

**【Program flow chart】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post295.png)

**【Main Program】**

```python
###################################Start############## #####################
...
sensor.set_pixformat(sensor.GRAYSCALE) #Set the picture format to grayscale
#Import image template
template1= image.Image("/1.pgm") #直行
template2 = image.Image("/2.pgm") #Turn to the right
template3 = image.Image("/3.pgm") #Turn left
template4 = image.Image("/4.pgm") #Parking to give way
template5 = image.Image("/5.pgm") #honk the horn
#Main loop
while (True):
    clock.tick()
    img = sensor.snapshot()
    flag=0
ratio=0
#Match 1.pgm (straight line) serial terminal print go, flag=1
    r1 = img.find_template(template1, 0.70, step=4, search=SEARCH_EX)
    if r1:
        img.draw_rectangle(r1,color=(255,0,0))
        print("go")
        flag=1
        img.draw_string(10, 10, "%.d"%flag)
#Match 2.pgm (turn to the right) serial terminal print right, flag=2
    r2 = img.find_template(template2, 0.70, step=4, search=SEARCH_EX)
    if r2:
        img.draw_rectangle(r2,color=(0,255,0))
        print("right")
        flag=2
        img.draw_string(10, 10, "%.d"%flag)
#Match 3.pgm (turn to the left) serial terminal print left, flag=3
    r3 = img.find_template(template3, 0.70, step=4, search=SEARCH_EX)
    if r3:
        img.draw_rectangle(r3,color=(255,0,0))
        print("left")
        flag=3
        img.draw_string(10, 10, "%.d"%flag)
#Match 4.pgm (stop to yield) serial terminal print stop, flag=4
    r4 = img.find_template(template4, 0.70, step=4, search=SEARCH_EX)
    if r4:
        img.draw_rectangle(r4,color=(255,255,0))
        print("stop")
        flag=4
        img.draw_string(10, 10, "%.d"%flag)
#Match 5.pgm (honk the horn) serial terminal print beep, flag=5
    r5 = img.find_template(template5, 0.70, step=4, search=SEARCH_EX)
    if r5:
        img.draw_rectangle(r5,color=(255,255,0))
        print("beep")
        flag=5
        img.draw_string(10, 10, "%.d"%flag)
###################################end############## #####################
```

##### ③ Lane recognition

 Mainly through the OpenMV module, the lane threshold is identified and tracked, the angle between the car and the lane center line is calculated by geometric calculation (left is positive, right is negative), and the true deviation of the car from the lane (quantifiable) is fed back, which is subsequently used PID control.

**【Program flow chart】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post296.png)

**【Main Program】**

```python
###################################Start############## #####################
...
sensor.set_pixformat(sensor.RGB565) # The picture format is set to RGB565 color map
road_threshold = [(23, 0, -45, 19, -31, 28)]; #黑线路
ROI = (0, 100, 320, 40)
...
#Omit to recognize the lane border function
#Offset angle calculation algorithm
def get_direction(left_blob, right_blob):
    # Calculate the deflection angle of the camera according to the three white parts on the left, middle and right in the image
    # ratio <0 Turn left, the car is on the right side of the lane
    # ratio> 0 Turn right, the car is on the left side of the lane

    MAX_WIDTH = 320
    # Adjust theta to set the proportion of the middle width, the higher the theta, the closer the ratio is to 0
    # Need to reset to the appropriate size according to the track width and camera height
    theta = 0.01
    # The b here is to prevent the divisor from being 0 and set a smaller value
    b = 3
    x1 = left_blob.x()-int(0.5 * left_blob.w()) #The center x value of the black line on the left
    x2 = right_blob.x() + int(0.5 * right_blob.w()) #The center x value of the black line on the right
# Lane information calculation
    w_left = x1 #Left lane width
    w_center = math.fabs(x2-x1) # Lane center x value
    w_right = math.fabs(MAX_WIDTH-x2) #The width outside the lane on the right
#Calculate the camera offset angle
    direct_ratio = (w_left + b + theta * w_center) / (w_left + w_right + 2 * b + 2 * theta * w_center)-0.5
#Return to camera offset angle
return direct_ratio
#Visual drawing function omitted
...
while(True): #Main loop
clock.tick() #Track the number of milliseconds that have passed between two snapshots()
    img = sensor.snapshot() #take a photo and return the image
    blobs = img.find_blobs(road_threshold, roi=ROI, merge=True);
    a=0;ratio=0;
    if blobs:
        left_blob, right_blob = get_top2_blobs(blobs)

        if(left_blob == None or right_blob == None):
            print("Out Of Range")
            continue
        else:
#Draw the left line of the lane
            img.draw_rectangle(left_blob.rect())
            img.draw_cross(left_blob.cx(), left_blob.cy())
#Draw the right line of the lane
            img.draw_rectangle(right_blob.rect())
            img.draw_cross(right_blob.cx(), right_blob.cy())
#Visual display of deflection angle
            direct_ratio = get_direction(left_blob, right_blob)
            draw_direct(img,direct_ratio)
            ratio=int(math.degrees(direct_ratio)) #The deflection angle is converted into a radian value
            img.draw_string(10, 10, "%.d"%ratio) #Frame buffer draws the deflection angle in real time
            print(ratio) #Serial terminal print deflection angle
    img.draw_rectangle(ROI,color=(255, 0, 0)) #Draw the region of interest
###################################end############## #####################
```

#### 2. Realization of remote control platform based on ESP8266

Mainly use the lighting technology-[Blinker](https://diandeng.tech/home) IoT platform to build the UI interface of the control APP, and call the control code of Blinker to realize the issuance of smart car control instructions and the upload of road condition data .

**【Remote Control Platform UI Interface】**

<img src="https://cdn.jsdelivr.net/gh/kelecn/images@master/post297.png" style="zoom: 25%;" />

**[UI configuration code]**

Directly use [diandeng.blinker](https://www.diandeng.tech/home) APP to import the configuration code to get the same UI layout as mine.

```
{¨config¨{¨headerColor¨¨transparent¨¨headerStyle¨¨dark¨¨background¨{¨img¨¨assets/img/headerbg.jpg¨¨isFull¨«}}¨dashboard¨|{¨type¨¨btn¨ ¨ico¨¨fad fa-arrow-alt-up¨¨mode¨É¨t0¨¨ forward¨¨t1¨¨ text 2¨¨bg¨Ì¨cols¨Ë¨rows¨Ë¨key¨¨btn-go¨ ´x´Ì´y´Ï¨speech¨|÷¨clr¨¨#076EEF¨}{ßAßBßC¨fad fa-arrow-alt-down¨ßEÉßF¨ back¨ßHßIßJÌßKËßLËßM¨btn-back¨´x´Ì´y´ ¤CßO|÷ßPßQ¨lstyle¨É}{ßAßBßC¨fad fa-arrow-alt-right¨ßEÉßF¨ turn right¨ßHßIßJÌßKËßLËßM¨btn-right¨´x´Ï´y´ÒßO|÷ßPßQßUÉ}{ßAßBß -arrow-alt-left¨ßEÉßF¨ Turn left¨ßHßIßJÌßKËßLËßM¨btn-left¨´x´É´y´ÒßO|÷ßPßQßUÉ}{ßAßBßC¨fad fa-power-off¨ßEÉßF¨Parking¨ßHßIßJÌßßË¨bt LËßË¨KËßLËßM¨btn-left ´x´Ï´y´ÏßO|÷ßPßQßUÉ}{ßA¨tex¨ßF¨😋Car remote monitoring system😋¨ßH´´ßJËßC´´ßKÍßLÊßM´´´x´Ë´y´ËßO|÷ßPßQßUÊ}{ßA¨ num¨ßF¨ obstacle distance¨ßC¨fad fa-route¨ßPßQ¨min¨É¨max¨¢1c¨uni¨¨cm¨ßJÉßKÍßLËßM¨num-distance¨´x´É´y´¤EßO|÷ßUÊ} {ßAßgßF¨ Trolley offset angle¨ßC¨fad fa-tachometer-alt-fast¨ßPßQßjÉßkº0ßl´º´ßJÉßKÍßLËßM¨num-angle¨´x´Í´y´¤EßO|÷ßUÊ}{ßAßgßF¨ traffic light (red 1 green 2)¨ßC¨fad fa-siren-on¨ßPßQßjÉßkËßl´´ßJÉßKËßLËßM¨num-led¨´x´É´y´ÏßO|÷ßUÉ}{ßA¨deb¨ßEÉßJÉßKÑßLÌßM¨debug¨´x´É´y´ÌßO|÷ßUÉ}{ßAßgßF¨WIFI signal¨ßC¨fad fa-signal-4¨ßP¨#389BEE¨ßjÉßkº0ßl¨dbm¨ßJÉßKËßLËß-wifi ´x´Ï´y´ÉßO|÷ßUÉ}{ßAßBßC¨fad fa-repeat-alt¨ßEÊßF¨ Autopilot mode¨ßHßIßJËßKËßLËßM¨btn-auto¨´x´Ì´y´ÒßO|÷ßPßQsUÉ}÷¨action |¦¨cmd¨¦¨switch¨‡¨text¨‡´on´¨ on?name¨¨off¨¨ off?name¨—÷¨triggers¨|{¨source¨ß16¨source_zh¨¨ switch state¨¨state ¨|´on´ß19÷¨state_zh¨|´open´´close´÷}÷}
```

**【Control instructions and monitoring data】**

| Name | Function of the function button/data receiving box | Data key name | Command |
| ------------ | ------------------------------- | ---- -------- | ---- |
| WiFi signal | Receive WiFi signal data | num-wifi | — |
| Traffic light data | Receive traffic light data (no 0, red 1, green 2) | num-led | — |
| Trolley offset angle | Receive trolley offset angle | num-angle | — |
| Obstacle distance | Receive obstacle distance data | num-distance | — |
| Stop | Issue a parking command | btn-stoping | 0 |
| Forward | Issue a forward command | btn-go | 1 |
| Turn right | Issue a right turn instruction | btn-right | 2 |
| Turn left | Issue a left turn command | btn-left | 3 |
| Back | Issue a back command | btn-back | 4 |
| Autopilot | Issuing an autopilot command | btn-auto | 5 |
| Debug | Display the original data format of the sent and received data | — | — |

**【Program flow chart】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post298.png)

**【Main Program】**

```c
/***********************************Start************* **********************/
...
int flag = 0; //Key flag bit
int l = 0; //Traffic light data
int a = 0; //Angle data
int d = 0; //distance data
int z = 0; //data parsed by json
BlinkerNumber Number0("num-wifi");//WIFI signal
BlinkerNumber Number1("num-led");//Traffic light signal
BlinkerNumber Number2("num-angle");//Angle signal
BlinkerNumber Number3("num-distance");//distance signal
BlinkerButton Button0("btn-stoping");//Stop state button
BlinkerButton Button1("btn-go");//forward state button
BlinkerButton Button2("btn-right");//Right turn state button
BlinkerButton Button3("btn-left");//Left turn state button
BlinkerButton Button4("btn-back");//Back state button
BlinkerButton Button5("btn-auto");//Automatic driving state button
...
/*Main loop*/
void loop()
{
    Blinker.run();
    Number0.print(WiFi.RSSI()); //Send signal strength
    usartEvent();//Serial port interrupt
    l=int(z/10000); //Analyze traffic light data
    a=int((z-10000*l)/100); //Analyze offset angle data
d=int(z-10000*l-100*a); //Analyze distance data
    Number1.print(l); //Send traffic light signal
    Number2.print(a); //Send angle signal
    Number3.print(d); //Send distance signal
//Send a control command, the light turns on and off, mainly to check whether the WiFi module receives data
    if(oState == false && digitalRead(LED_BUILTIN)== LOW)//light off
    {
      digitalWrite(LED_BUILTIN,HIGH);//light off
      Serial.print(flag); //Send command
    }
     else if(oState == true && digitalRead(LED_BUILTIN)== HIGH)//light on
     {
       digitalWrite(LED_BUILTIN,LOW);//The light is on
       Serial.print(flag); //Send command
     }
}
//Blinker initialization is omitted
//WiFi connection signal detection slightly
//STM32 data upload analysis slightly
...
/***********************************end************* **********************/
```

#### 3. Realization of unmanned control scheme of smart car

The smart car receives the ESP8266 control instructions and OpenMV road condition data, and controls the movement of the car according to these instruction data.

**【Program flow chart】**

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post299.png)

**【PID control algorithm】**

Regarding the PID adjustment of the DC motor, it is mainly used to realize the lane keeping function. Through the deflection angle returned by OpenMV, the PWM output of the motor is adjusted in real time to make the deflection angle $Y=50$​ (that is, the deflection angle between the trolley and the center line is 0, because 50 was added as a whole for the convenience of transmission). Therefore, the set value is set to 50, and the real-time return of the $Y$ value and 50 are used for the difference calculation to obtain the input deviation of the PID, and the real-time PWM value is returned through the positional PID. Regarding the [PID control algorithm](https://kelecn.top/posts/16358/), it was also introduced before, so I won’t go into details here.
$$
PWM=K_P\theta(t)+K_i\sum_{t=0}\theta(t)+K_d[\theta(t)-\theta(t-1)]
$$
Among them, $\theta(t)$​​is the difference between the offset angle data $Y$​​returned by OpenMV this time and 50, and $\theta(t-1)$​​is the previous $Y$​ The difference between ​​and 50.

![](https://cdn.jsdelivr.net/gh/kelecn/images@master/post299-1.png)

 **【Simulation environment】**

<img src="https://cdn.jsdelivr.net/gh/kelecn/images@master/post299-2.png" style="zoom: 33%;" />

**【Main Program】**

Slightly, see more : [OpenMV-autodrive](https://github.com/kelecn/OpenMV-autodrive)