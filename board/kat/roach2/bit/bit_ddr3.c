#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

static u32 ddr3_get_reg(u32 addr)
{
  return *((u32*)(CONFIG_SYS_FPGA_BASE + addr));
}


void ddr3_set_reg(u32 addr, u32 val)
{
  *((u32*)(CONFIG_SYS_FPGA_BASE + addr)) = val;
}

static int ddr3_mem_test(u32 flags)
{
  u32 i, j;
  u32 d, t;
  int ret = 0;

  /* walking zero */

  for (i=0; i < (BSP_DDR3_DLEN*32); i++){
    ddr3_set_reg(BSP_DDR3_REG_ADDR, i*8);
    for (j=0; j < BSP_DDR3_DLEN; j++) {
      ddr3_set_reg(BSP_DDR3_REG_WR(j), 0xffffffff);
    }

    ddr3_set_reg(BSP_DDR3_REG_WR(i/32), ~(1 << (31 - (i % 32))));
    //printf("setting %x to %x\n", i/32, ~(1 << (31 - (i % 32))));

    ddr3_set_reg(BSP_DDR3_REG_CTRL, BSP_DDR3_WR);
    ddr3_set_reg(BSP_DDR3_REG_CTRL, BSP_DDR3_RD);

    /* wait for rd flag to clear */
    for (j=0; j < 1000; j++){
      if (!(ddr3_get_reg(BSP_DDR3_REG_CTRL) & BSP_DDR3_RD)){
        break;
      }
      if (j == 999) {
        sprintf(bit_strerr, "timeout waiting for read to be completed");
        return -1;
      }
    }

    for (j=0; j < BSP_DDR3_DLEN; j++) {
      t = ddr3_get_reg(BSP_DDR3_REG_RD(j));

      if (i/32 == j)
        d = ~(1 << (31 - (i % 32)));
      else
        d = 0xffffffff;

      if (BSP_DDR3_IS_HALF(j))
        d &= 0xffff;

      if (d != t) {
        ret = -1;
        sprintf(bit_strerr, "walking 0 test, error on bit %d in data burst, %d on data bus", i, i%160 - 16);
        if (flags & 0x2)
          printf("%s\n", bit_strerr);
      }
    }

    if (flags & 0x1) {
      for (j=0; j < BSP_DDR3_DLEN; j++) {
        t = ddr3_get_reg(BSP_DDR3_REG_RD(j));
        printf("%08x",t);
      }
      printf("[%3d, %2d]\n", i, i%72);
    }
    
  }

  if (!(flags & 0x2)) {
    if (ret)
      return -1;
  }
  return ret;
}

static int ddr3_cal_test(u32 flags)
{
  u32 status = ddr3_get_reg(BSP_DDR3_REG_STATUS);
   if (!(status & BSP_DDR3_PHY_RDY)) {
     sprintf(bit_strerr, "phy ready bit stuck low");
     return -1;
   }
   return 0;
}

int bit_ddr3(int which, int subtest, u32 flags)
{
  switch (subtest){
  case 0:
    return ddr3_cal_test(flags);
  case 1:
    return ddr3_mem_test(flags);
  case 2:
    sprintf(bit_strerr, "test not implemented");
    return -1;
  default:
    sprintf(bit_strerr, "unsupport subtest");
    return -1;
  }
}
