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

#include "sensors.h"

#ifdef CONFIG_CMD_ROACH2_SENSORS

int dump_roach2_sensor_info(void)
{
  uchar buffer[1];


  if (i2c_read(R2_SENSOR_AD7414_U15_I2C_ADDR, R2_SENSOR_AD7414_TV, 1, buffer, 1) != 0) {
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_AD7414_U15_I2C_ADDR);
  }
  else
    printf("Ambient Temperature at inlet (U15) is\t:\t%d dC\n",(int)buffer[0]);
  
  if (i2c_read(R2_SENSOR_AD7414_U18_I2C_ADDR, R2_SENSOR_AD7414_TV, 1, buffer, 1) != 0) {
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_AD7414_U18_I2C_ADDR);
  }
  else
    printf("Ambient Temperature at outlet (U18) is\t:\t%d dC\n",(int)buffer[0]);

  if (i2c_read(R2_SENSOR_MAX6650_U13_I2C_ADDR, R2_SENSOR_MAX6650_TACH0,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX6650_U13_I2C_ADDR);
  }
  else
    printf("Fan Speed FPGA (U13)\t\t\t:\t%d rpm\n",((int)buffer[0])*30);
  
  if (i2c_read(R2_SENSOR_MAX6650_U17_I2C_ADDR, R2_SENSOR_MAX6650_TACH0,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX6650_U17_I2C_ADDR);
  }
  else
    printf("Fan Speed CHS Fan0 (U17)\t\t:\t%d rpm\n",((int)buffer[0])*30);
  
  if (i2c_read(R2_SENSOR_MAX6650_U21_I2C_ADDR, R2_SENSOR_MAX6650_TACH0,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX6650_U21_I2C_ADDR);
  }
  else
    printf("Fan Speed CHS Fan1 (U21)\t\t:\t%d rpm\n",((int)buffer[0])*30);
 
  if (i2c_read(R2_SENSOR_MAX6650_U26_I2C_ADDR, R2_SENSOR_MAX6650_TACH0,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX6650_U26_I2C_ADDR);
  } 
  else
    printf("Fan Speed CHS Fan2 (U26)\t\t:\t%d rpm\n",((int)buffer[0])*30);
 
  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX1,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("Remote Temp of PPC (U22)\t\t:\t%d dC\t",(int)buffer[0]);
  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX1_THIGH,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("T_HIGH: %d dC ",(int)buffer[0]);
  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX1_TLOW,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("T_LOW: %d dC",(int)buffer[0]);
  printf("\n");

  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX2,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("Remote Temp of FPGA (U22)\t\t:\t%d dC\t",(int)buffer[0]);
  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX2_THIGH,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("T_HIGH: %d dC ",(int)buffer[0]);
  if (i2c_read(R2_SENSOR_MAX1805_U22_I2C_ADDR, R2_SENSOR_MAX1805_TEMP_DX2_TLOW,1, buffer, 1) != 0){
    printf("cannot read from i2c device: %02x\n",R2_SENSOR_MAX1805_U22_I2C_ADDR);
  }
  else
    printf("T_LOW: %d dC",(int)buffer[0]);
  printf("\n");
  
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
    case 3:
      if (strcmp(argv[1],"rtc") == 0){
         printf("in rtc logic\n");
         return 0;
      }
    case 2:
      if (strcmp(argv[1], "dump") == 0){
        return dump_roach2_sensor_info();
      }
    default:
      printf("error: unrecognised command\n");
      printf("Usage:\n%s\n",cmdtp->usage);
      break;
  }

  return 1;
}

U_BOOT_CMD(
    roach2_sensors, 4, 1, do_roach2_sensors,
    "roach2_sensors [ dump | rtc [time] ]",
    "dump - dump roach2 sensor information\n"
    "roach2_sensors rtc [time] - set the real time clock to time [ms from 60ies]\n"
);
#endif

int sensors_config(void)
{
  printf("Sensors Config");
  
  return 0;
}

