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

#include "include/gpio.h"

#ifdef CONFIG_CMD_R2GPIO

const struct gpio_mapping gpio_map[GPIO_COUNT] = R2_GPIO_MAPPING;

static int do_roach2_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  int cmd = -1;
  int gpio = -1;
  int i;
  int val;

  if (argc <= 1) {
    cmd = 1;
  } else if (strcmp(argv[1], "get") == 0){
    if (argc == 2) {
      cmd = 1;
    } else if (argc == 3){
      cmd = 2;
    }
  } else if (strcmp(argv[1], "set") == 0 && argc == 4) {
    cmd = 3;
  }

  if (cmd == -1) {
    printf ("error: unrecogonized command\n");
    printf ("Usage:\n%s\n", cmdtp->usage);
    return 1;
  }

  if (cmd == 1) {
    for (i=0; i < GPIO_COUNT; i++) {
      printf("gpio: %s = %x\n",gpio_map[i].name, gpio_read_in_bit(gpio_map[i].index));
    }
    return 0;
  }

  if (cmd == 2 || cmd == 3){
    for (i=0; i < GPIO_COUNT; i++) {
      if (strcmp(gpio_map[i].name, argv[2]) == 0){
        gpio = i;
      }
    }
  }

  if (gpio < 0 || gpio >= GPIO_COUNT) {
    printf("error: %s is not a valid ROACH 2 gpio\n",argv[2]);
    printf("       choose one of the following:  \n");
    for (i=0; i < GPIO_COUNT; i++) {
      printf("%s",gpio_map[i].name);
      if (i != GPIO_COUNT - 1)
        printf(", ");
    }
    printf("\n");
    return 1;
  }
  if (cmd == 2) {
    printf("gpio: %s = %x\n",gpio_map[gpio].name, gpio_read_in_bit(gpio_map[gpio].index));
    return 0;
  }
  /* write */
  if (strcmp(argv[3],"0")==0){
    val = 0;
  } else if (strcmp(argv[3],"1")==0){
    val = 1;
  } else if (strcmp(argv[3],"i")==0){
    val = -1;
  } else if (strcmp(argv[3],"o")==0){
    val = -2;
  } else {
    printf("error: only values of 0,1,o and i are supported\n");
    return 1;
  }

  if (val >= 0) {
    gpio_write_bit(gpio_map[gpio].index, val);
    return 0;
  }

  gpio_config(gpio_map[gpio].index, val == -1 ? GPIO_IN : GPIO_OUT, GPIO_SEL, 1); 
  return 0;
}

U_BOOT_CMD(
	r2gpio,	4,	1,	do_roach2_gpio,
	"control roach2 GPIOs",
	"<set> <gpio> <val> - set the gpio val to '1', '0', 'i' (input) or 'o' (output)\n"
	"       <get> [gpio] - get all or a specific gpio value"
);
#endif /* CONFIG_CMD_R2GPIO */
