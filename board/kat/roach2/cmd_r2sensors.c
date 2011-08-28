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

#ifdef CONFIG_CMD_R2SENSORS

struct max16071_current cmon_defs[CMON_COUNT] = CURRENT_DEFS;
struct max16071_voltage vmon_defs[VMON_COUNT] = VOLTAGE_DEFS;
struct max16071_pgood pgood_defs[PGOOD_COUNT] = PGOOD_DEFS;

static int sensor_get_reg(u8 addr, u8 reg)
{
  uchar buffer[1];
  if (i2c_read(addr, reg, 1, buffer, 1) != 0) {
    printf("cannot read from i2c device: %02x\n", addr);
    return -1;
  }
  return buffer[0];
}

static int max16071_get_adcval(int which, u8 chan)
{
  int addr = which == VMON ? R2_VMON_IIC_ADDR : R2_CMON_IIC_ADDR;
  int ret = 0;
  int val;
  val = sensor_get_reg(addr, MAX16071_REG_ADCVAL_MSB(chan));
  if (val < 0) {
    return -1;
  }
  ret |= (val & 0xff) << 8;
  val = sensor_get_reg(addr, MAX16071_REG_ADCVAL_LSB(chan));
  if (val < 0) {
    return -1;
  }
  ret |= (val & 0xff);
  return ret;
}

static int max16071_get_cmon(int which)
{
  int addr = which == VMON ? R2_VMON_IIC_ADDR : R2_CMON_IIC_ADDR;
  int val;
  val = sensor_get_reg(addr, MAX16071_REG_CMON);
  if (val < 0) {
    return -1;
  }
  return val & 0xff;
}

static void vmon_print_vals(void)
{
  int i;
  int value;
  printf("Supply voltages:\n");
  for (i=0; i < VMON_COUNT; i++){
    value = max16071_get_adcval(vmon_defs[i].device, vmon_defs[i].src);
    if (value < 0) {
      printf("error reading supply %s\n", vmon_defs[i].name);
    } else {
      value = (((value * VMON_FULLSCALE * vmon_defs[i].gain) / ((2^16)-1))) / 1000;
      printf("supply %s: %d mV\n", vmon_defs[i].name, value);
    }
  }
}

static void cmon_print_vals(void)
{
  int i;
  int value;
  printf("Supply currents:\n");
  for (i=0; i < CMON_COUNT; i++){
    if (cmon_defs[i].src == CMON_SOURCE_EXTERNAL) {
      value = max16071_get_cmon(cmon_defs[i].device);
      if (value < 0) {
        printf("error reading supply %s\n", cmon_defs[i].name);
        continue;
      } else {
        value = (value * CMON_EXTERNAL_FULLSCALE *
                     cmon_defs[i].conductance)/(((2^8)-1) * cmon_defs[i].gain);
      }
    } else {
      value = max16071_get_adcval(cmon_defs[i].device, cmon_defs[i].src);
      if (value < 0) {
        printf("error reading supply %s\n", cmon_defs[i].name);
        continue;
      } else {
        value = (value * CMON_FULLSCALE *
                     cmon_defs[i].conductance)/(((2^16)-1) * cmon_defs[i].gain);
      }
    }
    printf("supply %s: %d mA\n", cmon_defs[i].name, value);
  }
}

static void pgood_print_vals(void)
{
  int i;
  int value;
  printf("Supply power-goods:\n");

  for (i=0; i < PGOOD_COUNT; i++){
    value = sensor_get_reg(pgood_defs[i].device, MAX16071_REG_GPIOI);
    if (value < 0) {
      printf("error reading supply %s\n", vmon_defs[i].name);
    } else {
      value &= (1 << pgood_defs[i].src);
      printf("supply %s: ", vmon_defs[i].name);
      if ((value && pgood_defs[i].level == PGOOD_ACTIVE_HIGH) ||
          (!value && pgood_defs[i].level != PGOOD_ACTIVE_HIGH))
        printf("good\n");
      else
        printf("bad\n");
    }
  }
}

/* vvvvv TODO: requires cleanup vvvvv */

void ambient_print_vals(u8 addr, char *descrip)
{
  uchar buffer[2];

  if (i2c_read(addr, R2_SENSOR_AD7414_TV, 1, buffer, 2) != 0) {
    printf("error getting ambient[%02x]\n", addr);
    return;
  }

  printf("ambient %s[%02x]:   %4d dC %s\n", descrip, addr, buffer[0],
                    buffer[1] & 0x18 ? "[ALARM]" : "");
}

void remote_print_vals(void)
{
  int value;
  u8 addr = R2_SENSOR_MAX1805_U22_I2C_ADDR;

  value = sensor_get_reg(addr, R2_SENSOR_MAX1805_TEMP_DX1);
  if (value < 0){
    printf("error getting remote temperature\n");
    return;
  }
  printf("PowerPC  [%02x]:   %4d dC", addr, value);
  value = sensor_get_reg(addr, R2_SENSOR_MAX1805_TEMP_RS2);
  printf("%s\n", value & 0xC0 ? "[ALARM]" : "");

  value = sensor_get_reg(addr, R2_SENSOR_MAX1805_TEMP_DX2);
  if (value < 0){
    printf("error getting remote temperature\n");
    return;
  }
  printf("Virtex6  [%02x]:   %4d dC", addr, value);
  value = sensor_get_reg(addr, R2_SENSOR_MAX1805_TEMP_RS2);
  printf("%s\n", value & 0x30 ? "[ALARM]" : "");
}

void fans_print_vals(u8 addr, char* descrip)
{
  int value;

  value = sensor_get_reg(addr, R2_SENSOR_MAX6650_TACH0);
  if (value < 0){
    printf("error getting fan speed\n");
    return;
  }
  printf("fan %s[%2x]:   %4d RPM", descrip, addr, value*30);
  value = sensor_get_reg(addr, R2_SENSOR_MAX6650_ALARMST);
  if (value < 0){
    printf("error getting fan speed\n");
    return;
  }
  printf("%s\n", value & 0x03 ? "[ALARM]" : "");
}

int dump_roach2_sensor_info(void)
{
 //vmon_print_vals();
 //cmon_print_vals();
 //pgood_print_vals();
  ambient_print_vals(R2_SENSOR_AD7414_U15_I2C_ADDR, " ");
  ambient_print_vals(R2_SENSOR_AD7414_U18_I2C_ADDR, " ");
  remote_print_vals();
  fans_print_vals(R2_SENSOR_MAX6650_U13_I2C_ADDR, "FPGA ");
  fans_print_vals(R2_SENSOR_MAX6650_U17_I2C_ADDR, "CHS0 ");
  fans_print_vals(R2_SENSOR_MAX6650_U21_I2C_ADDR, "CHS1 ");
  fans_print_vals(R2_SENSOR_MAX6650_U26_I2C_ADDR, "CHS2 ");
  
  return 0;
}

static int do_roach2_sensors(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  /*int i;
  printf("Arg count: %d\n",argc);

  for (i=0; i<argc; i++)
    printf("%d:%s\n",i,argv[i]);
*/
  
  switch (argc) { 
    case 2:
      if (strcmp(argv[1], "dump") == 0){
        return dump_roach2_sensor_info();
      }
    case 1:
      return dump_roach2_sensor_info();
      break;
    default:
      return 1;
  }

  return 1;
}

U_BOOT_CMD(
    r2sensor, 4, 1, do_roach2_sensors,
    "access roach2 sensors",
    "[dump] - dump roach2 sensor information"
);

#endif /* CONFIG_CMD_R2SENSORS */ 
