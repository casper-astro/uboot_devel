#ifndef _RTC_ROACH2_H_
#define _RTC_ROACH2_H_

/*DS1307 Real Time Clock*/
#define R2_RTC_DS1307_U11_I2C_ADDR   0x68
#define R2_RTC_DS1307_TIME           0x0
#define R2_RTC_DS1307_TIME_LEN       0x7
#define R2_RTC_DS1307_12HR           0x40

#define R2_RTC_DS1307_DAY(x)       ((x) == 0 ? "SUN" : (x) == 1 ? "MON" :\
(x) == 2 ? "TUE" : (x) == 3 ? "WED" :\
(x) == 4 ? "THU" : (x) == 5 ? "FRI" :\
(x) == 6 ? "SAT" : "ERR")

#endif /*define _SENSORS_ROACH2_H_*/
