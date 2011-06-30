#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

/************************** V6 Comms BIT *******************************/

#define TEST_LEN 4
int v6comm_scratchtest(u32 flags)
{
  u32 offset = CONFIG_SYS_FPGA_BASE;
  u32 test_data[TEST_LEN] = {0x55555555, 0xaaaaaaaa, 0x01234567, 0x89abcdef};
  u16 half_test_data[TEST_LEN*2] = {0x8081, 0x9293, 0xa4a5, 0xb6b7,
                                    0xc0c1, 0xd2d3, 0xe4e5, 0xf6f7};
  u32 val;
  u16 halfval;
  int i;

  for (i = 0; i < TEST_LEN; i++) {
    *((volatile u32 *) (offset + BSP_REG_SCRATCH(i))) = test_data[i];
  }

  for (i = 0; i < TEST_LEN; i++) {
    val = *((volatile u32 *)( offset + BSP_REG_SCRATCH(i)));
    if (val != test_data[i]) {
      sprintf(bit_strerr, "scratchpad readback failure at %8x - got %x, expected %x",
                                offset + BSP_REG_SCRATCH(i), val, test_data[i]);
      return -1;
    }
  }

  for (i = 0; i < TEST_LEN*2; i++) {
    *((volatile u16 *) (offset + BSP_REG_SCRATCH(0) + i*2)) = half_test_data[i];
  }

  for (i = 0; i < TEST_LEN*2; i++) {
    halfval = *((volatile u16 *) (offset + BSP_REG_SCRATCH(0) + i*2));
    if (halfval != half_test_data[i]) {
      sprintf(bit_strerr, "scratchpad readback failure at %8x - got %x, expected %x",
                              offset + BSP_REG_SCRATCH(0) + i*2, halfval, half_test_data[i]);
      return -1;
    }
  }
  return 0;
}

int bit_v6comm(int which, int subtest, u32 flags)
{
  int ret = 0;
  int i;
  u32 offset = CONFIG_SYS_FPGA_BASE;
  u32 bid, maj, min, rcs;

  switch (subtest){
  case 0:
    bid = *((volatile u32 *) (offset + BSP_REG_BOARDID));
    maj = *((volatile u32 *) (offset + BSP_REG_REVMAJ));
    min = *((volatile u32 *) (offset + BSP_REG_REVMIN));
    rcs = *((volatile u32 *) (offset + BSP_REG_REVRCS));

    if (flags & 0x1)
      printf("ID: %x; REV: %x.%x.%x\n",bid,maj,min,rcs);

    if (bid != BSP_BOARDID) {
      sprintf(bit_strerr, "board ID check failed; got %x, expected %x", bid, BSP_BOARDID);
      return -1;
    }
    if (maj == 0) {
      sprintf(bit_strerr, "unsupport major revision: %x", maj);
      return -1;
    }
    if (rcs == 0) {
      sprintf(bit_strerr, "rcs revision not supported: %x", rcs);
      return -1;
    }
    break;
  case 1:
    for (i=0;i < 256; i++) {
      ret = v6comm_scratchtest(flags);
      if (ret)
        return -1;
    }
    break;
  default:
    sprintf(bit_strerr, "unsupport subtest");
    ret = -1;
    break;
  }

  return ret;
}


