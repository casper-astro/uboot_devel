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

#include "cmd_roach2.h"

#ifdef CONFIG_CMD_ROACH2_SMAP
/**** ROACH 2 SelectMAP Programming ****/

//#define DEBUG 0

int smap_check_bitstream(u32 addr)
{
  u32 sync_val_bin = *((u32 *)(addr + SMAP_SYNC_OFFSET_BIN));
  u32 sync_val_bit = *((u32 *)(addr + SMAP_SYNC_OFFSET_BIT));

  if (sync_val_bin == SMAP_SYNC_VALUE){
#ifdef DEBUG
    printf("info: detected a valid .bin\n");
#endif
    return 0;
  } else if (sync_val_bit == SMAP_SYNC_VALUE){
#ifdef DEBUG
    printf("info: detected a valid .bit\n");
#endif
    /* return the offset into addr from which the real file starts */
    return SMAP_SYNC_OFFSET_BIT - SMAP_SYNC_OFFSET_BIN;
  } else {
#ifdef DEBUG
    printf("info: invalid bitstream sync symbol read; got %x \n", sync_val_bin);
    printf("or    invalid bitstream sync symbol read; got %x \n", sync_val_bit);
#endif
    return -1;
  }
}

int smap_program(u32 addr, unsigned int length)
{
  int i;
  int offset = 0;
  volatile u32 *src;
  volatile u32 *dst;

  if ((offset = smap_check_bitstream(addr)) < 0){
    printf("error: invalid bitstream detected\n");
    return -1;
  }

  /* configure initn as output*/
  gpio_config(GPIO_SMAP_INITN, GPIO_OUT, GPIO_SEL, GPIO_OUT_1);

  /* RD_WR_N set to 0 [WR], INIT_N to 1, PROG_B to 1 */
  gpio_write_bit(GPIO_SMAP_RDWRN, 0);
  gpio_write_bit(GPIO_SMAP_INITN, 1);
  gpio_write_bit(GPIO_SMAP_PROGN, 1);


  /* INIT_N to 0, PROG_N to 0 */
  for (i=0; i < 32; i++){ /* Hold for at least 350ns */
    gpio_write_bit(GPIO_SMAP_INITN, 0);
    gpio_write_bit(GPIO_SMAP_PROGN, 0);
  }

  /* clear prog_n */
  gpio_write_bit(GPIO_SMAP_PROGN, 1);

  /* configure initn as input */
  gpio_config(GPIO_SMAP_INITN, GPIO_IN, GPIO_SEL, GPIO_OUT_0);


  /* here we need to wait for initn to be driven to one by the v6 */
  for (i=0; i < SMAP_INITN_WAIT + 1; i++){
    if (gpio_read_in_bit(GPIO_SMAP_INITN))
      break;
    if (i == SMAP_INITN_WAIT){
      printf("error: SelectMAP programming failed, init_n stuck low\n");
      return -1;
    }
  }

  src = (u32 *)(addr + offset);
  dst = (u32 *)(CONFIG_SYS_SMAP_BASE);

  for(i = 0; i < length; i+=4){
#ifdef DEBUG
    if (i < (16 * 4))
      printf("%d: loaded smap data %8x\n", i, *src);
#endif
#ifdef DEBUG
    if (i > length - (16 * 4))
      printf("%d: loaded smap data %8x\n", i, *src);
#endif
    *dst = *src;
    src++;
  }

  for (i=0; i < SMAP_DONE_WAIT + 1; i++){
    if (gpio_read_in_bit(GPIO_SMAP_DONE)){
      return 0;
    }
  }

  printf("error: SelectMAP programming failed, done stuck low\n");
  return 1;
}

static int do_roach2_smap(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  ulong addr, length;

  length = SMAP_IMAGE_SIZE;

  if (argc < 1) {
    printf ("Usage:\n%s\n", cmdtp->usage);
    return 1;
  }

  addr = simple_strtoul(argv[1], NULL, 16);
  if(argc > 2){
     length = simple_strtoul(argv[2], NULL, 10);
  }

  printf("source %lx (%lu bytes)\n", addr, length);

  /*
   */

  if (smap_program(addr, length)){
    printf("error: SelectMAP configuration failed\n");
    return -1;
  }

  printf("info: SelectMAP configuration succeeded\n");
  return 0;
}

U_BOOT_CMD(
	roach2_smap,	3,	1,	do_roach2_smap,
	"roach2_smap - program fpga using selectmap interface\n",
	"address [length]\n"
        "        - source address with optional length\n"
);
#endif /* CONFIG_CMD_ROACH2_SMAP */


#ifdef CONFIG_CMD_ROACH2_DEBUG
static int do_roach2_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  u32 val;

  mfebc(EBC0_CFG,val);
  printf("ebc:            cfg=0x%08x\n", val);

  mfebc(PB0AP,val);
  printf("ebc: cs0 [flash] ap=0x%08x\n", val);
  mfebc(PB0CR,val);
  printf("ebc: cs0 [flash] cr=0x%08x\n", val);
  mfebc(PB1AP,val);
  printf("ebc: cs1 [fpga]  ap=0x%08x\n", val);
  mfebc(PB1CR,val);
  printf("ebc: cs1 [fpga]  cr=0x%08x\n", val);

  mfebc(PB2AP,val);
  printf("ebc: cs2 [cpld]  ap=0x%08x\n", val);
  mfebc(PB2CR,val);
  printf("ebc: cs2 [cpld]  cr=0x%08x\n", val);
  mfebc(PB3AP,val);
  printf("ebc: cs3 [smap]  ap=0x%08x\n", val);
  mfebc(PB3CR,val);
  printf("ebc: cs3 [smap]  cr=0x%08x\n", val);
  mfebc(PBEAR,val);
  printf("ebc:            ear=0x%08x\n", val);
  mfebc(PBESR0,val);
  printf("ebc:           esr0=0x%08x\n", val);
  mfebc(PBESR1,val);
  printf("ebc:           esr1=0x%08x\n", val);

  printf("GPIO0_OR           :   0x%08lx\n", in32(GPIO0_OR     ));
  printf("GPIO0_TCR          :   0x%08lx\n", in32(GPIO0_TCR    ));
  printf("GPIO0_OSRL         :   0x%08lx\n", in32(GPIO0_OSRL   ));
  printf("GPIO0_OSRH         :   0x%08lx\n", in32(GPIO0_OSRH   ));
  printf("GPIO0_TSRL         :   0x%08lx\n", in32(GPIO0_TSRL   ));
  printf("GPIO0_TSRH         :   0x%08lx\n", in32(GPIO0_TSRH   ));
  printf("GPIO0_ISR1L        :   0x%08lx\n", in32(GPIO0_ISR1L  ));
  printf("GPIO0_ISR1H        :   0x%08lx\n", in32(GPIO0_ISR1H  ));
  printf("GPIO0_ISR2L        :   0x%08lx\n", in32(GPIO0_ISR2L  ));
  printf("GPIO0_ISR2H        :   0x%08lx\n", in32(GPIO0_ISR2H  ));
  printf("GPIO0_ISR3L        :   0x%08lx\n", in32(GPIO0_ISR3L  ));
  printf("GPIO0_ISR3H        :   0x%08lx\n", in32(GPIO0_ISR3H  ));
                       
  printf("GPIO1_OR           :   0x%08lx\n", in32(GPIO1_OR     ));
  printf("GPIO1_TCR          :   0x%08lx\n", in32(GPIO1_TCR    ));
  printf("GPIO1_OSRL         :   0x%08lx\n", in32(GPIO1_OSRL   ));
  printf("GPIO1_OSRH         :   0x%08lx\n", in32(GPIO1_OSRH   ));
  printf("GPIO1_TSRL         :   0x%08lx\n", in32(GPIO1_TSRL   ));
  printf("GPIO1_TSRH         :   0x%08lx\n", in32(GPIO1_TSRH   ));
  printf("GPIO1_ISR1L        :   0x%08lx\n", in32(GPIO1_ISR1L  ));
  printf("GPIO1_ISR1H        :   0x%08lx\n", in32(GPIO1_ISR1H  ));
  printf("GPIO1_ISR2L        :   0x%08lx\n", in32(GPIO1_ISR2L  ));
  printf("GPIO1_ISR2H        :   0x%08lx\n", in32(GPIO1_ISR2H  ));
  printf("GPIO1_ISR3L        :   0x%08lx\n", in32(GPIO1_ISR3L  ));
  printf("GPIO1_ISR3H        :   0x%08lx\n", in32(GPIO1_ISR3H  ));


  return 0;
}

U_BOOT_CMD(
	roach2_debug,	3,	1,	do_roach2_debug,
	"roach2_debug - print some useful debug info\n",
	"[level]\n"
        "        - optional debug level\n"
);
#endif /* CONFIG_CMD_ROACH2_DEBUG */

#ifdef CONFIG_CMD_ROACH2_GPIO
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
	roach2_gpio,	4,	1,	do_roach2_gpio,
	"roach2_gpio - control roach2 GPIOs\n",
	"roach2_gpio set gpio val\n"
	"roach2_gpio get [gpio]\n"
);
#endif /* CONFIG_CMD_ROACH2_GPIO */
