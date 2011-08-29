/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/ppc4xx-ebc.h>
#include <i2c.h>

#include "include/sensors.h"

struct max6650_config fan_config_fpga = {
  .speed = 0x2a, //5700 RPM
  .config = R2_SENSOR_MAX6650_CFG_SCALE4 | R2_SENSOR_MAX6650_CFG_12V |
                                            R2_SENSOR_MAX6650_CFG_CLOOP,
  .gpio_config = 0xf0 | R2_SENSOR_MAX6650_GPIO0_ALERT,
  .alarm_config = R2_SENSOR_MAX6650_ALARM_EN
};

struct max6650_config fan_config_chs0 = {
  .speed = 0x2a, //5700 RPM
  .config = R2_SENSOR_MAX6650_CFG_SCALE4 | R2_SENSOR_MAX6650_CFG_12V |
                                            R2_SENSOR_MAX6650_CFG_FULLON,
  .gpio_config = 0xf0 | R2_SENSOR_MAX6650_GPIO1_FULLON | R2_SENSOR_MAX6650_GPIO0_ALERT,
  .alarm_config = 0
};

struct max6650_config fan_config_chs1 = {
  .speed = 0x2a, //5700 RPM
  .config = R2_SENSOR_MAX6650_CFG_SCALE4 | R2_SENSOR_MAX6650_CFG_12V |
                                            R2_SENSOR_MAX6650_CFG_FULLON,
  .gpio_config = 0xf0 | R2_SENSOR_MAX6650_GPIO1_FULLON | R2_SENSOR_MAX6650_GPIO0_ALERT,
  .alarm_config = 0
};

struct max6650_config fan_config_chs2 = {
  .speed = 0x2a, //5700 RPM
  .config = R2_SENSOR_MAX6650_CFG_SCALE4 | R2_SENSOR_MAX6650_CFG_12V |
                                            R2_SENSOR_MAX6650_CFG_FULLON,
  .gpio_config = 0xf0 | R2_SENSOR_MAX6650_GPIO1_FULLON | R2_SENSOR_MAX6650_GPIO0_ALERT,
  .alarm_config = 0
};

struct max1805_config remote_config = {
  .config = 0x0,
  .local_max = 45,
  .local_min = 0,
  .remote_max = {120, 80}, /* PPC [invalid faults need to be ignored], FPGA */
  .remote_min = {0, 0}
};

struct ad7414_config ambient0_config = {
  .config = AD7414_CFG_IICFLTR | AD7414_CFG_ALRMRST,
  .max = 40,
  .min = 0
};

struct ad7414_config ambient1_config = {
  .config = AD7414_CFG_IICFLTR | AD7414_CFG_ALRMRST,
  .max = 40,
  .min = 0
};

struct max16071_config vmon_config = {
  .en_config = MAX16071_EN_SW,
  /* first is fault all others input */
  .gpio_function = 0x0, // MAX16071_GPIO_FUNC(MAX16071_GPIO_FUNC_FAULTANY, 0x0),
  /* first is open drain, dont care for others */
  .gpio_out_type = MAX16071_GPIO_OTYPE_ODRAIN,
  /* disable all over voltage checks */
  .ov = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
  /* disable all under voltage checks */
  .uv = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
  .suv = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
  .cmon_config = MAX16071_CMON_EN | MAX16071_CMON_RANGE16V | MAX16071_CMON_GAIN24,
  .chan_config = {
      MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6,
      MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6
    },
  };

struct max16071_config cmon_config = {
  .en_config = MAX16071_EN_SW,
  /* first is fault all others input */
  .gpio_function = 0x0, // MAX16071_GPIO_FUNC(MAX16071_GPIO_FUNC_FAULTANY, 0x0),
  /* first is open drain, dont care for others */
  .gpio_out_type = MAX16071_GPIO_OTYPE_ODRAIN,
  /* disable all over voltage checks */
  .ov = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
  /* disable all under voltage checks */
  .uv = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
  .suv = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
  .cmon_config = MAX16071_CMON_EN | MAX16071_CMON_RANGE16V | MAX16071_CMON_GAIN24,
  .chan_config = {
      MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6,
      MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6, MAX16071_CHAN_FS5V6
    },
  };

static int sensor_write(u8 addr, u8 reg, u8 val)
{
  if (i2c_write(addr, reg, 1, &val, 1) != 0) {
    printf("cannot write to i2c device: %02x@%02x\n", addr, reg);
    return -1;
  }
  return 0;
}

static int sensor_get_reg(u8 addr, u8 reg)
{
  uchar buffer[1];
  if (i2c_read(addr, reg, 1, buffer, 1) != 0) {
    printf("cannot read from i2c device: %02x\n", addr);
    return -1;
  }
  return buffer[0];
}

int max16071_config(int addr, struct max16071_config* maxc)
{
  int i;
  u8 scratch=0;
  if (sensor_write(addr, MAX16071_REG_GPIOOTYPE, maxc->gpio_out_type)) {
    return -1;
  }

  if (sensor_write(addr, MAX16071_REG_GPIOCONF0, (maxc->gpio_function >> 0) & 0xff)) {
    return -1;
  }
  if (sensor_write(addr, MAX16071_REG_GPIOCONF1, (maxc->gpio_function >> 8) & 0xff)) {
    return -1;
  }
  if (sensor_write(addr, MAX16071_REG_GPIOCONF2, (maxc->gpio_function >> 16) & 0xff)) {
    return -1;
  }

  for (i=0; i < MAX16071_CHANCNT; i++) {
    if (sensor_write(addr, MAX16071_REG_SUV(i), maxc->suv[i])) {
      return -1;
    }
    if (sensor_write(addr, MAX16071_REG_OV(i), maxc->ov[i])) {
      return -1;
    }
    if (sensor_write(addr, MAX16071_REG_UV(i), maxc->uv[i])) {
      return -1;
    }
  }

  if (sensor_write(addr, MAX16071_REG_CMONCONFIG, maxc->cmon_config)) {
    return -1;
  }

  for (i=0; i < MAX16071_CHANCNT; i++) {
    if (i%4 == 0)
      scratch=0;
    scratch |= (maxc->chan_config[i] & 0x3) << (2*(i%4));
    if (i%4 == 3) {
      if (sensor_write(addr, MAX16071_REG_CHANGAIN(i/4), scratch)) {
        return -1;
      }
    }
  }

  if (sensor_write(addr, MAX16071_REG_EN, maxc->en_config)) {
    return -1;
  }

  return 0;
}

int ad7414_config(u8 addr, struct ad7414_config *config)
{
  u8 buffer[2];

  if (sensor_write(addr, AD7414_REG_CFG, config->config)) {
    return -1;
  }
  if (sensor_write(addr, AD7414_REG_MIN, config->min)) {
    return -1;
  }
  if (sensor_write(addr, AD7414_REG_MAX, config->max)) {
    return -1;
  }

  /* need to read values to clear fault flags */
  if (i2c_read(addr, R2_SENSOR_AD7414_TV, 1, buffer, 2) != 0) {
    return -1;
  }

  return 0;
}

int max1805_config(u8 addr, struct max1805_config *config)
{
  u8 buffer[1];
  int i;
  if (sensor_write(addr, MAX1805_REG_WC, config->config)) {
    return -1;
  }
  if (sensor_write(addr, MAX1805_REG_LOCAL_MIN_WR, config->local_min)) {
    return -1;
  }
  if (sensor_write(addr, MAX1805_REG_LOCAL_MAX_WR, config->local_max)) {
    return -1;
  }
  for (i=0; i < MAX1805_REMOTE_COUNT; i++) {
    if (sensor_write(addr, MAX1805_REG_REMOTE_MAX_WR(i), config->remote_max[i])) {
      return -1;
    }
    if (sensor_write(addr, MAX1805_REG_REMOTE_MIN_WR(i), config->remote_min[i])) {
      return -1;
    }
  }

  /* need to read values to clear fault flags */
  if (i2c_read(addr, R2_SENSOR_MAX1805_TEMP_RS2, 1, buffer, 1) != 0) {
    return -1;
  }
  if (i2c_read(addr, R2_SENSOR_MAX1805_TEMP_RS1, 1, buffer, 1) != 0) {
    return -1;
  }

  return 0;
}

int max6650_config(u8 addr, struct max6650_config* config)
{
  if (sensor_write(addr, R2_SENSOR_MAX6650_SPEED, config->speed)) {
    return -1;
  }
  if (sensor_write(addr, R2_SENSOR_MAX6650_ALARM, config->alarm_config)) {
    return -1;
  }
  if (sensor_write(addr, R2_SENSOR_MAX6650_GPIO, config->gpio_config)) {
    return -1;
  }
  if (sensor_write(addr, R2_SENSOR_MAX6650_CONFIG, config->config)) {
    return -1;
  }
  return 0;
}

int sensors_config(void)
{
  int ret = 0;
  int fail = 0;
  printf("Sensors Config");
  ret = max16071_config(R2_VMON_IIC_ADDR, &vmon_config);
  if (ret) {
    printf("max16071: vmon config failed\n");
    fail = 1;
  }
  ret = max16071_config(R2_CMON_IIC_ADDR, &cmon_config);
  if (ret) {
    printf("max16071: cmon config failed\n");
    fail = 1;
  }

  ret = ad7414_config(R2_SENSOR_AD7414_U15_I2C_ADDR, &ambient0_config);
  if (ret) {
    printf("ad7414: ambient0 config failed\n");
    fail = 1;
  }
  ret = ad7414_config(R2_SENSOR_AD7414_U18_I2C_ADDR, &ambient1_config);
  if (ret) {
    printf("ad7414: ambient1 config failed\n");
    fail = 1;
  }

  ret = max1805_config(R2_SENSOR_MAX1805_U22_I2C_ADDR, &remote_config);
  if (ret) {
    printf("max1805: remote temp config failed\n");
    fail = 1;
  }

  ret = max6650_config(R2_SENSOR_MAX6650_U13_I2C_ADDR, &fan_config_fpga);
  if (ret) {
    printf("max6650: fpga fan config failed\n");
    fail = 1;
  }

  ret = max6650_config(R2_SENSOR_MAX6650_U17_I2C_ADDR, &fan_config_chs0);
  if (ret) {
    printf("max6650: chs0 config failed\n");
    fail = 1;
  }
  
  ret = max6650_config(R2_SENSOR_MAX6650_U21_I2C_ADDR, &fan_config_chs1);
  if (ret) {
    printf("max6650: chs1 config failed\n");
    fail = 1;
  }
  
  ret = max6650_config(R2_SENSOR_MAX6650_U26_I2C_ADDR, &fan_config_chs2);
  if (ret) {
    printf("max6650: chs2 config failed\n");
    fail = 1;
  }
  
  return fail;
}
