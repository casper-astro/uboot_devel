#ifndef _SENSORS_ROACH2_H_
#define _SENSORS_ROACH2_H_

#ifdef CONFIG_CMD_ROACH2_SENSORS

/*AD7414 Ambient temperature sensor*/
#define R2_SENSOR_AD7414_U15_I2C_ADDR   0x4C
#define R2_SENSOR_AD7414_U18_I2C_ADDR   0x4E
#define R2_SENSOR_AD7414_TV             0
#define R2_SENSOR_AD7414_CR             1
#define R2_SENSOR_AD7414_THIGH          2
#define R2_SENSOR_AD7414_TLOW           3

/*MAX6650 FAN Speeds*/
#define R2_SENSOR_MAX6650_U13_I2C_ADDR  0x48
#define R2_SENSOR_MAX6650_U17_I2C_ADDR  0x4B
#define R2_SENSOR_MAX6650_U21_I2C_ADDR  0x1B
#define R2_SENSOR_MAX6650_U26_I2C_ADDR  0x1F
#define R2_SENSOR_MAX6650_TACH0         0x0C

/*DS1307 Real Time Clock*/
/*TODO: Not in i2c probe*/
#define R2_SENSOR_DS1307_U11_I2C_ADDR   0x68

/*MAX1805 Remote temperature for V6 and PPC*/
#define R2_SENSOR_MAX1805_U22_I2C_ADDR  0x18
#define R2_SENSOR_MAX1805_TEMP_DX1      0x01
#define R2_SENSOR_MAX1805_TEMP_DX2      0x02


#endif /*define CONFIG_CMD_RAOCH2_SENSORS*/
#endif /*define _SENSORS_ROACH2_H_*/
