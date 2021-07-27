#ifndef __WARE_H
#define	__WARE_H

void Wave_SRD_Init(void);        //超声波模块的初始化

void Wave_SRD_Strat(void);         //超声波模块本省的触发条件，大于10us的触发

extern float Distance;

#endif /* __UltrasonicWave_H */

