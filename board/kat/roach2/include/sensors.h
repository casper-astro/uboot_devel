#ifndef _SENSORS_ROACH2_H_
#define _SENSORS_ROACH2_H_


/*********** MAX6650 defines *************/

struct max6650_config {
  u8 speed;
  u8 config;
  u8 gpio_config;
  u8 alarm_config;
};

#define R2_SENSOR_MAX6650_U13_I2C_ADDR  0x48
#define R2_SENSOR_MAX6650_U17_I2C_ADDR  0x4B
#define R2_SENSOR_MAX6650_U21_I2C_ADDR  0x1B
#define R2_SENSOR_MAX6650_U26_I2C_ADDR  0x1F
#define R2_SENSOR_MAX6650_TACH0         0x0C
#define R2_SENSOR_MAX6650_ALARMST       0x0a

#define R2_SENSOR_MAX6650_SPEED         0x0
#define R2_SENSOR_MAX6650_CONFIG        0x2
#define R2_SENSOR_MAX6650_ALARM         0x8
#define R2_SENSOR_MAX6650_GPIO          0x4

#define R2_SENSOR_MAX6650_CFG_SCALE4    0x2
#define R2_SENSOR_MAX6650_CFG_12V       0x8
#define R2_SENSOR_MAX6650_CFG_CLOOP     0x20
#define R2_SENSOR_MAX6650_CFG_FULLON    0x00

#define R2_SENSOR_MAX6650_ALARM_EN      0x3
#define R2_SENSOR_MAX6650_GPIO1_FULLON  (0x1 << 2)
#define R2_SENSOR_MAX6650_GPIO0_ALERT   (0x1)

/******** MAX1668/MAX1805 defines *********/

#define R2_SENSOR_MAX1805_U22_I2C_ADDR      0x18
#define R2_SENSOR_MAX1805_TEMP_DX1          0x01
#define R2_SENSOR_MAX1805_TEMP_DX1_THIGH    0x0A
#define R2_SENSOR_MAX1805_TEMP_DX1_TLOW     0x0B
#define R2_SENSOR_MAX1805_TEMP_DX2          0x02
#define R2_SENSOR_MAX1805_TEMP_DX2_THIGH    0x0C
#define R2_SENSOR_MAX1805_TEMP_DX2_TLOW     0x0D
#define R2_SENSOR_MAX1805_TEMP_RS1          0x05
#define R2_SENSOR_MAX1805_TEMP_RS2          0x06

#define MAX1805_REMOTE_COUNT 2

#define MAX1805_REG_WC 0x12 
#define MAX1805_REG_LOCAL_MAX_WR 0x13 
#define MAX1805_REG_LOCAL_MIN_WR 0x14 
#define MAX1805_REG_REMOTE_MAX_WR(x) (0x15 + (x)*2)
#define MAX1805_REG_REMOTE_MIN_WR(x) (0x16 + (x)*2)

struct max1805_config {
  u8 config;
  u8 local_max;
  u8 local_min;
  u8 remote_max[MAX1805_REMOTE_COUNT];
  u8 remote_min[MAX1805_REMOTE_COUNT];
};

/************ AD7417 defines **************/

#define R2_SENSOR_AD7414_U15_I2C_ADDR   0x4C
#define R2_SENSOR_AD7414_U18_I2C_ADDR   0x4E

#define R2_SENSOR_AD7414_TV             0x00
#define R2_SENSOR_AD7414_CR             0x01
#define R2_SENSOR_AD7414_THIGH          0x02
#define R2_SENSOR_AD7414_TLOW           0x03

#define AD7414_REG_CFG 0x01
#define AD7414_REG_MAX 0x02
#define AD7414_REG_MIN 0x03

#define AD7414_CFG_IICFLTR (1<<6)

#define AD7414_CFG_ALRMRST (1<<3)

struct ad7414_config {
  u8 config;
  u8 max;
  u8 min;
};

/*********** MAX16071 defines *************/

#define R2_VMON_IIC_ADDR  0x50
#define R2_CMON_IIC_ADDR  0x51

#define MAX16071_REG_ADCVAL_MSB(x) ((x)*2)
#define MAX16071_REG_ADCVAL_LSB(x) ((x)*2 + 1)

#define MAX16071_REG_CMON 0x18
#define MAX16071_REG_GPIOI 0x1e

#define MAX16071_REG_GPIOCONF0 0x3f
#define MAX16071_REG_GPIOCONF1 0x40
#define MAX16071_REG_GPIOCONF2 0x41
#define MAX16071_REG_GPIOOTYPE 0x42

#define MAX16071_REG_SUV(x) (0x48 + 3*(x))
#define MAX16071_REG_OV(x) (0x49 + 3*(x))
#define MAX16071_REG_UV(x) (0x4a + 3*(x))

#define MAX16071_REG_CMONCONFIG 0x47

#define MAX16071_CHANCNT 8
struct max16071_config {
  u32 gpio_function;
  u8 gpio_out_type;
  u8 ov[MAX16071_CHANCNT];
  u8 uv[MAX16071_CHANCNT];
  u8 suv[MAX16071_CHANCNT];
  u8 cmon_config;
  u8 chan_config[MAX16071_CHANCNT];
};

#define MAX16071_CHAN_FS5V6 0x0
#define MAX16071_CHAN_FS1V4 0x2

#define MAX16071_GPIO_FUNC_INPUT 0x0
#define MAX16071_GPIO_FUNC_FAULTANY 0x4
#define MAX16071_GPIO_FUNC(n,x) (((n) & 0x7) << ((x)*3))
#define MAX16071_GPIO_OTYPE_ODRAIN 0x1

#define MAX16071_CMON_EN 0x1
#define MAX16071_CMON_RANGE16V (0x1 << 1)  
#define MAX16071_CMON_GAIN24 (0x10 << 2)

#define MAX16071_REG_CHANGAIN(x) (0x43 + (x))

struct max16071_current {
  char* name;
  u8 device;
  u8 src;
  u16 gain; /* 1 == 1 */
  u16 conductance; /* in siemons */
};

struct max16071_voltage {
  char* name;
  u8 device;
  u8 src;
  u16 gain; /* 1000 == 1 */
};

struct max16071_pgood {
  char* name;
  u8 device;
  u8 src;
  u8 level;
};

#define VMON 0
#define CMON 1

#define CMON_SOURCE_EXTERNAL 255

#define CMON_FULLSCALE 1400
#define CMON_EXTERNAL_FULLSCALE 58

#define CMON_COUNT 7
#define CURRENT_DEFS { {"1V0", CMON, 4, 1000, 4000}, \
                       {"1V5", CMON, 3, 1000, 1000}, \
                       {"1V8", CMON, 2, 500, 500}, \
                       {"2V5", CMON, 1, 200, 200}, \
                       {"3V3", CMON, 0, 500, 500}, \
                       {" 5V", CMON, CMON_SOURCE_EXTERNAL, 1, 500}, \
                       {"12V", VMON, CMON_SOURCE_EXTERNAL, 1, 200},}

#define VMON_FULLSCALE 5600
#define VMON_COUNT 8
#define VOLTAGE_DEFS { {"   1V0", VMON, 0, 1000}, \
                       {"   1V5", VMON, 1, 1000}, \
                       {"   1V8", VMON, 2, 1000}, \
                       {"   2V5", VMON, 3, 1000}, \
                       {"   3V3", VMON, 4, 1000}, \
                       {"    5V", VMON, 5, 1000}, \
                       {"   12V", VMON, 6, 2959}, \
                       {"3V3AUX", VMON, 7, 1000},}

#define PGOOD_ACTIVE_HIGH 1
#define PGOOD_COUNT 3
#define PGOOD_DEFS { {"    ATX", VMON, 1, PGOOD_ACTIVE_HIGH}, \
                     {"MGT_1V2", VMON, 2, PGOOD_ACTIVE_HIGH}, \
                     {"MGT_1V0", VMON, 3, PGOOD_ACTIVE_HIGH},}

#endif /*define _SENSORS_ROACH2_H_*/
