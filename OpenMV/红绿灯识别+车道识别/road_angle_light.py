# 红绿灯识别+道路识别 - By: 谢青桦 @kelecn - 周四 一月 29  2021

import sensor, image, time, math
from pyb import UART
import json
import ustruct

#white_threshold_01 = ((95, 100, -18, 3, -8, 4));  #白色阈值
light_threshold = [(59, 100, 26, 127, -128, 127),(59, 100, -128, -40, -128, 127)]; #0无数据 1红灯 2绿灯 4黄灯(59, 100, -128, 127, 28, 127)
road_threshold = [(23, 0, -45, 19, -31, 28)]; #黑线道路
ROI = (0, 100, 320, 40)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()

uart = UART(3,115200)   #定义串口3变量
uart.init(115200, bits=8, parity=None, stop=1) # init with given parameters

#寻找红绿灯
def find_max(blobs):    #定义寻找色块面积最大的函数
    max_size=0
    for blob in blobs:
        if blob.pixels() > max_size:
            max_blob=blob
            max_size = blob.pixels()
    return max_blob

#发送数据
def sending_data(LED_color,ratio):
    global uart;
    #frame=[0x2C,18,cx%0xff,int(cx/0xff),cy%0xff,int(cy/0xff),0x5B];
    #data = bytearray(frame)
    data = ustruct.pack("<bbii",               #格式为俩个字符一个整型一个单精度浮点数
                   0xAA,                       #帧头1
                   0xAE,                       #帧头2
                   int(LED_color), # up sample by 4   #数据1 红绿灯数据
                   int(ratio)) # up sample by 4    #数据2 道路数据
    uart.write(data);   #必须要传入一个字节数组

def recive_data():
    global uart
    if uart.any():
        tmp_data = uart.readline();
        print(tmp_data)

#寻找道路
# 选择排序的属性， 这里我们选取的是色块宽度
def get_blob_value(blob):
    return blob.w()

# 选择最大的两个黑线色块
def compare_blob(blob1, blob2):
    comp_result = get_blob_value(blob1) - get_blob_value(blob2)

    if comp_result > 3:
        return 1
    elif comp_result < -3:
        return -1
    else:
        return 0

def get_direction(left_blob, right_blob):
    # 根据左中右三块白色部分，计算出角度值
    # ratio < 0 左拐
    # ratio > 0 右拐

    MAX_WIDTH = 320
    # 调节theta来设置中间宽度的比重， theta越高ratio越靠近0
    # 需要根据赛道宽度与摄像头高度重新设定到合适大小
    theta = 0.01
    # 这里的b是为了防止除数是0的情况发生， 设定一个小一点的值
    b = 3
    x1 = left_blob.x() - int(0.5 * left_blob.w())
    x2 = right_blob.x() + int(0.5 * right_blob.w())

    w_left = x1
    w_center = math.fabs(x2 - x1)
    w_right = math.fabs(MAX_WIDTH - x2)

    direct_ratio = (w_left + b + theta * w_center) / (w_left + w_right + 2 * b + 2 * theta * w_center) - 0.5

    return direct_ratio

def get_top2_blobs(blobs):
    # 找到最大的两个色块， 返回左色块与右色块
    for blob in blobs:
        pass
        #print(blob)
        # img.draw_rectangle(blob.rect())

    if len(blobs) < 2:
        # 已偏离轨道
        return (None, None)

    top_blob1 = blobs[0]
    top_blob2 = blobs[1]

    if compare_blob(top_blob1, top_blob2) == -1:
        top_blob1, top_blob2 = top_blob2, top_blob1


    for i in range(2, len(blobs)):
        if compare_blob(blobs[i], top_blob1) == 1:
            top_blob2 = top_blob1
            top_blob1 = blobs[i]
        elif compare_blob(blobs[i], top_blob2) == 1:
            top_blob2 = blobs[i]

    if top_blob1.cx() > top_blob2.cx():
        return (top_blob2, top_blob1)
    else:
        return (top_blob1, top_blob2)

def draw_direct(img, direct_ratio):
    # 可视化方向(左右) 标识及其幅度
    img.draw_circle(160, 80, 5)
    img.draw_line((160, 80, int(160 + direct_ratio * 200), 80))

#mainloop
while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    #  pixels_threshold=100, area_threshold=100
    blobs1 = img.find_blobs(light_threshold, area_threshold=150);
    blobs2 = img.find_blobs(road_threshold, roi=ROI, merge=True);
    cx=0;cy=0;a=0;ratio=0;LED_color=0;
    if blobs1:
        #如果找到了目标颜色
        max_b = find_max(blobs1)
        # Draw a rect around the blob.
        img.draw_rectangle(max_b[0:4]) # rect
        #用矩形标记出目标颜色区域
        img.draw_cross(max_b[5], max_b[6]) # cx, cy
        img.draw_cross(160, 120) # 在中心点画标记
        #在目标颜色区域的中心画十字形标记
        cx=max_b[5];
        cy=max_b[8];

        img.draw_line((160,120,cx,cy), color=(127));
        #img.draw_string(160,120, "(%d, %d)"%(160,120), color=(127));
        img.draw_string(cx, cy, "(%d, %d)"%(cx,cy), color=(127));
        LED_color=cy;
    #sending_data(cx,cy); #发送数据
    #recive_data();
    #print(cx,cy);
    #time.sleep(1000)
    #目标区域找到直线
    if blobs2:
        left_blob, right_blob = get_top2_blobs(blobs2)

        if(left_blob == None or right_blob == None):
            print("Out Of Range")
            continue
        else:
            #print("-------")
            #print("left blob")
            #print(left_blob)
            #print("right blob")
            #print(right_blob)
            img.draw_rectangle(left_blob.rect())
            img.draw_cross(left_blob.cx(), left_blob.cy())

            img.draw_rectangle(right_blob.rect())
            img.draw_cross(right_blob.cx(), right_blob.cy())

            direct_ratio = get_direction(left_blob, right_blob)
            #img.draw_string(10, 10, "%.2f"%direct_ratio)
            draw_direct(img,direct_ratio)
            #print(direct_ratio)
            #if(direct_ratio>0):
                #ratio=2
            #elif(direct_ratio==0):
                #ratio=1
            #else:
                #ratio=0
                #注意计算得到的是弧度值
                #将计算结果的弧度值转化为角度值
                #ratio=0:中间
                # ratio < 0 左拐
                # ratio > 0 右拐
            ratio=int(math.degrees(direct_ratio)) #传输正数，越偏右越小，越偏左越大，50应该就是中间
            img.draw_string(10, 10, "%.d"%ratio)
            print(LED_color,ratio)
            sending_data(LED_color,ratio) #发送数据
            #recive_data();
    img.draw_rectangle(ROI,color=(255, 0, 0))


#pack各字母对应类型
#x   pad byte        no value            1
#c   char            string of length 1  1
#b   signed char     integer             1
#B   unsigned char   integer             1
#?   _Bool           bool                1
#h   short           integer             2
#H   unsigned short  integer             2
#i   int             integer             4
#I   unsigned int    integer or long     4
#l   long            integer             4
#L   unsigned long   long                4
#q   long long       long                8
#Q   unsilong long   long                8
#f   float           float               4
#d   double          float               8
#s   char[]          string              1
#p   char[]          string              1
#P   void *          long
#  该程序只能输送x轴坐标，与需要的颜色坐标，如果输出x、y、颜色
#的话 单片机显示屏特别慢
