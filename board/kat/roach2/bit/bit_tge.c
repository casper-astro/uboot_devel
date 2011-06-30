#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

static u16 tge_reg_get_short(int which, u32 reg)
{
  u32 offset = CONFIG_SYS_FPGA_BASE + BSP_TGE_OFFSET(which);
  return *((u16*)(offset + BSP_TGE_REG_OFFSET + reg));
}

static int tge_check_phy_status(int which, int delay)
{
  u16 idle_start, idle_end;
  idle_end = tge_reg_get_short(which, BSP_TGE_REG_STLINK);
  idle_start = tge_reg_get_short(which, BSP_TGE_REG_STIDLE);
  if ((idle_end & 0x7c) != 0x7c) {
    sprintf(bit_strerr,"xaui link down (status = %x)", idle_end);
    return -1;
  }
  udelay(delay);
  idle_end = tge_reg_get_short(which, BSP_TGE_REG_STIDLE);
  if (idle_end != idle_start) {
    sprintf(bit_strerr,"xaui decoding errors detected");
    return -1;
  }
  return 0;
}

int bit_tge(int which, int subtest, u32 flags) {
  switch (subtest){
  case 0: 
    return tge_check_phy_status(which, 1000000);
    break;
  case 1: 
    sprintf(bit_strerr,"not implemented");
    return -1;
    break;
  default: 
    sprintf(bit_strerr,"subtest not supported");
    return -1;
    break;
  }
  return 0;
}
