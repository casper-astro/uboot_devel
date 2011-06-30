#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

/***************************** ZDOK BIT *******************************/

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


int bit_zdok(int which, int subtest, u32 flags)
{
  int i;
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

  for (i=0; i < 80; i++){
    /* bit on which the expected activity would be */
    expecting = (i/10)%2 ? i - 10: i + 10;
    error = 0;

    zdok_set_bit(which, i, BSP_GPIO_OUTPUT, BSP_GPIO_VAL_0);

#if 0
    /* dump all the bits */
    for (j=0;j < 80; j++){
      printf("%d",zdok_get_bit(which,j));
    }
#endif

    if (zdok_get_bit(which, expecting)){
      error = 1;
    }

    zdok_set_bit(which, i, BSP_GPIO_OUTPUT, BSP_GPIO_VAL_1);

    if (!zdok_get_bit(which, expecting)){
      error = 1;
    }

    zdok_set_bit(which, i, BSP_GPIO_INPUT, BSP_GPIO_VAL_0);

    if (error) {
      sprintf(bit_strerr, "connectivity error on zdok%d between pins %d & %d", which, i, expecting);
      ret = 1;
      break;
    }
  }

  return ret;
}

