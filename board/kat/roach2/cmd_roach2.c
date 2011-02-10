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

#include <asm/ppc4xx-gpio.h>

#include "cmd_roach2.h"

#ifdef CONFIG_CMD_ROACH2_SMAP
/**** ROACH 2 SelectMAP Programming ****/

#define DEBUG 1

int smap_check_bitstream(u32 *addr)
{
  u32 sync_val = addr[SMAP_SYNC_OFFSET];
  if (sync_val == SMAP_SYNC_VALUE){
    debug("info: detected a valid .bin\n");
    return 0;
  } else {
    debug("info: invalid bistream sync symbol read; got %x \n", sync_val);
    return -1;
  }
}

int smap_program(u32 *addr, unsigned int length)
{
  int i;
  volatile u32 *src;
  volatile u32 *dst;

  if (smap_check_bitstream(addr)){
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

  src = (addr);
  dst = (u32 *)(CONFIG_SYS_SMAP_BASE);

  for(i = 0; i < length; i+=4){
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

  if (smap_program((u32 *)addr, length)){
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

