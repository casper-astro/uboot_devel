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

#include "roach2_bsp.h"

#ifdef CONFIG_CMD_ROACH2_TEST

void zdok_set_bit(int which, u32 bit, u8 dir, u8 value)
{
  u32 offset;
  u32 prev;

  offset = CONFIG_SYS_FPGA_BASE + (which ? BSP_ZDOK1_OFFSET : BSP_ZDOK0_OFFSET);

  /* First set the value */
  offset += (bit / 32) * 16 + 4;

  prev = *((volatile u32 *) (offset));

  *((volatile u32 *) (offset)) = (prev & (~(1 << (bit%32)))) |
                                ((value == BSP_GPIO_VAL_1) << (bit%32));

  /* Then the direction */
  offset += 4;

  prev = *((volatile u32 *) (offset));

  *((volatile u32 *) (offset)) = (prev & (~(1 << (bit%32)))) |
                                ((dir == BSP_GPIO_OUTPUT) << (bit%32));
}

int zdok_get_bit(int which, u32 bit)
{
  u32 offset;
  u32 val;
  offset = CONFIG_SYS_FPGA_BASE + (which ? BSP_ZDOK1_OFFSET : BSP_ZDOK0_OFFSET);
  offset += (bit / 32) * 16;

  val = *((volatile u32 *) (offset));

  return (val & (1 << (bit%32))) != 0;
}


int bit_zdok(int which)
{
  int i,j;
  u8 expecting;
  u8 error;
  int ret=0;

  /*
  
  ZDOK loopback test: 

  */

  for (i=0; i < 80; i++){
    /* set all to 0 outputs */
    zdok_set_bit(which, i, BSP_GPIO_INPUT, BSP_GPIO_VAL_1);
  }

  printf("I0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x00)));
  printf("I1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x10)));
  printf("I2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x20)));

  printf("O0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x04)));
  printf("O1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x14)));
  printf("O2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x24)));

  printf("E0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x08)));
  printf("E1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x18)));
  printf("E2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x28)));

  for (i=0; i < 80; i++){
    /* bit on which the expected activity would be */
    expecting = (i/10)%2 ? i - 10: i + 10;
    error = 0;

    zdok_set_bit(which, i, BSP_GPIO_OUTPUT, BSP_GPIO_VAL_0);

    for (j=0;j < 80; j++){
      printf("%d",zdok_get_bit(which,j));
    }

    if (zdok_get_bit(which, expecting)){
      error = 1;
    }

    zdok_set_bit(which, i, BSP_GPIO_OUTPUT, BSP_GPIO_VAL_1);

    if (!zdok_get_bit(which, expecting)){
      error = 1;
    }

    zdok_set_bit(which, i, BSP_GPIO_INPUT, BSP_GPIO_VAL_0);

    if (error) {
      printf("e\n");
      ret = 1;
    } else {
      printf("\n");
    }

  }

  printf("I0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x00)));
  printf("I1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x10)));
  printf("I2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x20)));

  printf("O0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x04)));
  printf("O1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x14)));
  printf("O2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x24)));

  printf("E0: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x08)));
  printf("E1: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x18)));
  printf("E2: %08x\n", *((volatile u32 *)(CONFIG_SYS_FPGA_BASE + BSP_ZDOK0_OFFSET + 0x28)));
  return ret;
}



static int do_roach2_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  bit_zdok(0);
  bit_zdok(1);
  return 0;
}


U_BOOT_CMD(
	roach2_test,	3,	1,	do_roach2_test,
	"roach2_test - run through roach-2 built-in tests\n",
	"" ""
);
#endif /* CONFIG_CMD_ROACH2_TEST */
