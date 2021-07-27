# 交通标识识别 - By: 谢青桦 @kelecn - 周四 三月 25  2021
# Template Matching Example - Normalized Cross Correlation (NCC)
#
# This example shows off how to use the NCC feature of your OpenMV Cam to match
# image patches to parts of an image... expect for extremely controlled enviorments
# NCC is not all to useful.
#
# WARNING: NCC supports needs to be reworked! As of right now this feature needs
# a lot of work to be made into somethin useful. This script will reamin to show
# that the functionality exists, but, in its current state is inadequate.

import time, sensor, image, math
from image import SEARCH_EX, SEARCH_DS
from pyb import UART
import json
import ustruct

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
# Max resolution for template matching with SEARCH_EX is QQVGA
sensor.set_framesize(sensor.QQVGA)
# You can set windowing to reduce the search image.
#sensor.set_windowing(((640-80)//2, (480-60)//2, 80, 60))
sensor.set_pixformat(sensor.GRAYSCALE)

uart = UART(3,115200)   #定义串口3变量
uart.init(115200, bits=8, parity=None, stop=1) # init with given parameters

#发送数据
def sending_data(LED_color,ratio):
    global uart;
    #frame=[0x2C,18,cx%0xff,int(cx/0xff),cy%0xff,int(cy/0xff),0x5B];
    #data = bytearray(frame)
    data = ustruct.pack("<bbii",               #格式为俩个字符一个整型一个单精度浮点数
                   0xAA,                       #帧头1
                   0xAE,                       #帧头2
                   int(flag), # up sample by 4   #数据1 交通标识数据
                   int(ratio)) # up sample by 4    #数据2 道路数据
    uart.write(data);   #必须要传入一个字节数组

def recive_data():
    global uart
    if uart.any():
        tmp_data = uart.readline();
        print(tmp_data)

# Load template.
# Template should be a small (eg. 32x32 pixels) grayscale image.
template1 = image.Image("/1.pgm")
template2 = image.Image("/2.pgm")
template3 = image.Image("/3.pgm")
template4 = image.Image("/4.pgm")
template5 = image.Image("/5.pgm")

clock = time.clock()

# Run template matching
while (True):
    clock.tick()
    img = sensor.snapshot()
    flag=0
    ratio=0
    # find_template(template, threshold, [roi, step, search])
    # ROI: The region of interest tuple (x, y, w, h).
    # Step: The loop step used (y+=step, x+=step) use a bigger step to make it faster.
    # Search is either image.SEARCH_EX for exhaustive search or image.SEARCH_DS for diamond search
    #
    # Note1: ROI has to be smaller than the image and bigger than the template.
    # Note2: In diamond search, step and ROI are both ignored.
    r1 = img.find_template(template1, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
    if r1:
        img.draw_rectangle(r1,color=(255,0,0))
        print("go")
        flag=1
        img.draw_string(10, 10, "%.d"%flag)
        sending_data(flag,ratio)
    r2 = img.find_template(template2, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
    if r2:
        img.draw_rectangle(r2,color=(0,255,0))
        print("right")
        flag=2
        img.draw_string(10, 10, "%.d"%flag)
        sending_data(flag,ratio)
    r3 = img.find_template(template3, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
    if r3:
        img.draw_rectangle(r3,color=(255,0,0))
        print("left")
        flag=3
        img.draw_string(10, 10, "%.d"%flag)
        sending_data(flag,ratio)
    r4 = img.find_template(template4, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
    if r4:
        img.draw_rectangle(r4,color=(255,255,0))
        print("stop")
        flag=4
        img.draw_string(10, 10, "%.d"%flag)
        sending_data(flag,ratio)
    r5 = img.find_template(template5, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
    if r5:
        img.draw_rectangle(r5,color=(255,255,0))
        print("beep")
        flag=5
        img.draw_string(10, 10, "%.d"%flag)
        sending_data(flag,ratio)

    #print(clock.fps())
