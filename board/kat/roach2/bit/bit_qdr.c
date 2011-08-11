#include <common.h>
#include <net.h>

#include <asm/processor.h>
#include <asm/ppc4xx.h>

#include "../include/bit.h"
#include "../include/fpga.h"

/***************************** QDR  *******************************/

/******* CAL ********/

static void qdr_set_reg(int which, u32 reg, u32 val)
{
  u32 offset = CONFIG_SYS_FPGA_BASE;
  switch (which){
     case 0:
       offset += BSP_QDR0CONF_OFFSET;
       break;
     case 1:
       offset += BSP_QDR1CONF_OFFSET;
       break;
     case 2:
       offset += BSP_QDR2CONF_OFFSET;
       break;
     default:
       offset += BSP_QDR3CONF_OFFSET;
  }
  offset += reg;
  *((volatile u32 *) (offset)) = val;
}

static u32 qdr_get_reg(int which, u32 reg)
{
  u32 offset = CONFIG_SYS_FPGA_BASE;
  switch (which){
     case 0:
       offset += BSP_QDR0CONF_OFFSET;
       break;
     case 1:
       offset += BSP_QDR1CONF_OFFSET;
       break;
     case 2:
       offset += BSP_QDR2CONF_OFFSET;
       break;
     default:
       offset += BSP_QDR3CONF_OFFSET;
  }
  offset += reg;
  return *((volatile u32 *) (offset));
}

static void qdr_dll_reset(int which)
{
  u32 val;
  val = qdr_get_reg(which, BSP_QDRCONF_REG_CTRL);
  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL,
    BSP_QDRCONF_DLL_RESET);
  udelay(1000);
  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL,
    BSP_QDRCONF_DLL_RUN);
  udelay(1000);
}

/* increment(dir=1) or decrement(dir=0) the dll on selected data bit */
static int qdr_phy_tick_dly(int which, int dir)
{
  int i, ret=0;
  u32 val;
  qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_DLY_EN |
                           (dir ? BSP_QDRCONF_DLY_INC : BSP_QDRCONF_DLY_DEC));
  
  for (i=0; i < 1000; i++){
    val = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
    if (val & BSP_QDRCONF_DAT_RDY)
      break;

    if (i==999) {
      sprintf(bit_strerr, "calibration data done stuck low");
      ret = 1;
    }
  }
  return ret;
}

#define STABLE_THRESHOLD 4

#define EDGE_CLEARANCE 7

/*
  The QDR bit alignment algorithm goes something like this:
  We control the delay on a qdr data bit through 32 subtests of 72ps delay.
  The data is toggling every cycle. We increment the delay until we see a bit
  transition. We then choose a safe place to delay the data where we are far
  enough away from the bit edge (EDGE_CLEARANCE). We also take care to sample
  the data many times (done in the controller) so we know it is stable. We consider
  the EDGE_CLEARANCE to be applied to the last adjacent stable data point. 

  There are three case which are illustrated below:

  111BB1B2BBB22222222222222222222
                   ^         
  11111111111111111BB1B2BBB222222
            ^                
  1111111111111111111111111111111
         ^                


  We also need to do half bit alignment. So if we end up on 1's we can delay
  the bit by a whole cycle to get 2's.
*/

static int qdr_align_bit(int which, int bit, u32 flag)
{
  int i;
  u32 dreg;
  int val;
  int is_stable;
  int ret=0;

  u8 bit_line[32];
  u8 stable_count = 0;
  u8 baddies = 0;
  int first_val = 0;
  int prev_val = 0;

  int cal_pos = EDGE_CLEARANCE; /* default value if no transition is detected */

  qdr_set_reg(which, BSP_QDRCONF_REG_BITINDEX, (u32)bit);
  qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_DLY_RST);
  udelay(10);

  /* ensure that the status register is valid */
  qdr_phy_tick_dly(which, 1);
  qdr_phy_tick_dly(which, 0);

  for (i=0; i < 32; i++) {
    dreg = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
    is_stable = dreg & 0x00000100;
    val = dreg % 4;
    bit_line[i] = is_stable && (val == 1 || val == 2) ? val : 0xb;
    qdr_phy_tick_dly(which, 1);
  }

  for (i=0; i < 32; i++) {
    if (bit_line[i] != 0xb){
      if (!first_val){
        first_val = bit_line[i];
      }
      stable_count++;

      if (bit_line[i] != prev_val)
        stable_count = 0;
         
      if (first_val != bit_line[i] && stable_count >= STABLE_THRESHOLD){
        if (i - STABLE_THRESHOLD + EDGE_CLEARANCE >= 31){
          /* in this case the calibration point is before the bit transition */
          cal_pos = i - baddies - STABLE_THRESHOLD - EDGE_CLEARANCE;
          break;
        } else {
          /* in this case the calibration point is after the bit transition */
          cal_pos = i - STABLE_THRESHOLD + EDGE_CLEARANCE;
          break;
        }
      }
    } else {
      if (first_val){
        baddies++;
      }
      stable_count = 0;
    }
    prev_val = bit_line[i];
  }

  /* dump the bit */
  if (flag & 0x1) {
    for (i=0; i < 32; i++) {
      switch (bit_line[i]){
      case 1:
        printf("1");
        break;
      case 2:
        printf("2");
        break;
      default:
        printf("x");
      } 
    }
  }

  /* reset the delay elements to 0 offset */
  qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_DLY_RST);
  udelay(10);

  /* set the delay to cal_pos */
  for (i=0; i < cal_pos; i++) {
    qdr_phy_tick_dly(which, 1);
  }

  /* now get half bit alignment */
  udelay(10);
  dreg = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
  is_stable = dreg & 0x00000100;
  val = dreg % 4;

  if (val == 1){
    qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_ALIGN_EN);
  }

  if (!is_stable || val == 0 || val == 3){
    sprintf(bit_strerr, "calibrated data bit unstable or has bad value");
    ret = 1;
  } else {
    if (flag & 0x1) {
      printf("\n");
      for (i=0; i <= 31; i++) {
        if (i == cal_pos) 
          printf("^");
        else
          printf(" ");
      }
      printf(" [%02d]", cal_pos);
      //debug only
      printf(" val = %d", val);
    }
  }
  if (flag & 0x1)
    printf("\n");

  return ret;
}

static void qdr_dump_bit(int which, int bit)
{
  int i;
  u32 dreg;
  int is_stable;
  int val;
  qdr_set_reg(which, BSP_QDRCONF_REG_BITINDEX, (u32)bit);
  qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_DLY_RST);

  /* ensure that the status register is valid */
  qdr_phy_tick_dly(which, 1);
  qdr_phy_tick_dly(which, 0);

  printf("%2d: ", bit);
  for (i=0; i < 32; i++) {
    dreg = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
    is_stable = dreg & 0x00000100;
    val = dreg % 4;

    if (is_stable) {
      switch (val){
        case 1:
          printf("1");
          break;
        case 2:
          printf("2");
          break;
        default:
          printf("e");
      }
    } else {
      printf("x");
    }
    /* increase delay by one tap */
    qdr_phy_tick_dly(which, 1);
  }
  printf("\n");
}

static int qdr_phy_cal(int which, u32 flags)
{
  int i;
  int ret=0;
  u32 dreg;

  qdr_dll_reset(which);

  /* enable calibration logic */
  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL, BSP_QDRCONF_CAL_EN | BSP_QDRCONF_DLL_RUN);

  for (i=0; i < 1000; i++){
    dreg = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
    if (dreg & BSP_QDRCONF_CAL_RDY)
      break;
      
    if (i==999){
      sprintf(bit_strerr, "cal state machine not ready");
      return -1;
    }
  }

  for (i=0; i < 36; i++) {
    ret = qdr_align_bit(which, i, flags);
    if (ret)
      break;
  }

  /* disable calibration logic */
  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL, BSP_QDRCONF_DLL_RUN);

  return ret;
}

/******* MEM ********/

static void qdr_mem_set_reg(int which, u32 reg, u32 val)
{
  u32 offset = CONFIG_SYS_FPGA_BASE;
  switch (which){
     case 0:
       offset += BSP_QDR0_OFFSET;
       break;
     case 1:
       offset += BSP_QDR1_OFFSET;
       break;
     case 2:
       offset += BSP_QDR2_OFFSET;
       break;
     default:
       offset += BSP_QDR3_OFFSET;
  }
  offset += reg;
  *((volatile u32 *) (offset)) = val;
}

static u32 qdr_mem_get_reg(int which, u32 reg)
{
  u32 offset = CONFIG_SYS_FPGA_BASE;
  switch (which){
     case 0:
       offset += BSP_QDR0_OFFSET;
       break;
     case 1:
       offset += BSP_QDR1_OFFSET;
       break;
     case 2:
       offset += BSP_QDR2_OFFSET;
       break;
     default:
       offset += BSP_QDR3_OFFSET;
  }
  offset += reg;
  return *((volatile u32 *) (offset));
}

static int qdr_mem_test(int which, u32 flags)
{
  u32 i, j;
  u32 d, t;
  int ret = 0;
  d = qdr_mem_get_reg(which, BSP_QDRM_REG_STAT);

  if (!(d & BSP_QDRM_PHYRDY)) {
    sprintf(bit_strerr, "qdr phy not ready");
    return -1;
  }

  if ((d & BSP_QDRM_CALFAIL)) {
    sprintf(bit_strerr, "qdr phy calibration failed");
    return -1;
  }
  /* walking zero */

  for (i=0; i < 144; i++){
    qdr_mem_set_reg(which, BSP_QDRM_REG_A, i);

    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 0, 0xffffffff);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 4, 0xffffffff);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 8, 0xffffffff);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 12, 0xffffffff);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 16, 0xffffffff);

    qdr_mem_set_reg(which, BSP_QDRM_REG_D + (BSP_QDRM_DLEN - 1 - (i/32))*4, ~(1 << (i % 32)));

    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_WREN);

    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_RDEN);
    /* wait for rd flag to clear */
    for (j=0; j < 1000; j++){
      if (!(qdr_mem_get_reg(which, BSP_QDRM_REG_CTRL) & BSP_QDRM_RDEN)){
        break;
      }
      if (j == 999) {
        sprintf(bit_strerr, "timeout waiting for read to be completed");
        return -1;
      }
    }

    for (j=0; j < BSP_QDRM_DLEN; j++) {
      t = qdr_mem_get_reg(which, BSP_QDRM_REG_Q + (j << 2));

      if (j == (BSP_QDRM_DLEN - 1 - i/32))
        d = ~(1 << (i % 32));
      else
        d = 0xffffffff;

      if (j == 0)
        d &= 0xffff;

      if (d != t) {
        ret = -1;
        sprintf(bit_strerr, "walking 0 test, error on bit %d in data burst, %d on data bus", i, i%36);
        if (flags & 0x2)
          printf("%s\n", bit_strerr);
      }
    }

    if (flags & 0x1) {
      for (j=0; j < BSP_QDRM_DLEN; j++) {
        t = qdr_mem_get_reg(which, BSP_QDRM_REG_Q + (j << 2));
        printf("%08x",t);
      }
      printf("[%3d, %2d]\n", i, i%36);
    }
    
  }

  if (!(flags & 0x2)) {
    if (ret)
      return -1;
  }

  /* walking one */
  for (i=0; i < 144; i++){
    qdr_mem_set_reg(which, BSP_QDRM_REG_A, i);

    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 0, 0);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 4, 0);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 8, 0);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 12, 0);
    qdr_mem_set_reg(which, BSP_QDRM_REG_D + 16, 0);

    qdr_mem_set_reg(which, BSP_QDRM_REG_D + (BSP_QDRM_DLEN - 1 - (i/32))*4, (1 << (i % 32)));

    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_WREN);

    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_RDEN);
    /* wait for rd flag to clear */
    for (j=0; j < 1000; j++){
      if (!(qdr_mem_get_reg(which, BSP_QDRM_REG_CTRL) & BSP_QDRM_RDEN)){
        break;
      }
      if (j == 999) {
        sprintf(bit_strerr, "timeout waiting for read to be completed");
        return -1;
      }
    }

    for (j=0; j < BSP_QDRM_DLEN; j++) {
      t = qdr_mem_get_reg(which, BSP_QDRM_REG_Q + (j << 2));
      if (j == (BSP_QDRM_DLEN - 1 - i/32))
        d = (1 << (i % 32));
      else
        d = 0;

      if (d != t) {
        ret = -1;
        sprintf(bit_strerr, "walking 1 test, error on bit %d in data burst, %d on data bus", i, i%36);
        if (flags & 0x2)
          printf("%s\n", bit_strerr);
      }
    }

    if (flags & 0x1) {
      for (j=0; j < BSP_QDRM_DLEN; j++) {
        t = qdr_mem_get_reg(which, BSP_QDRM_REG_Q + (j << 2));
        printf("%08x",t);
      }
      printf("[%3d, %2d]\n", i, i%36);
    }
    
  }
  if (ret)
    return -1;


  for (i=0; i < BSP_QDRM_DEPTH; i++){
    qdr_mem_set_reg(which, BSP_QDRM_REG_A, i);

    d = (i % 2) ? (~(i / 2)) : (i/2);

    for (j=0; j < BSP_QDRM_DLEN; j++){
      qdr_mem_set_reg(which, BSP_QDRM_REG_D + (j << 2), d);
    }
    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_WREN);
  }

  for (i=0; i < BSP_QDRM_DEPTH; i++){
    qdr_mem_set_reg(which, BSP_QDRM_REG_A, i);

    qdr_mem_set_reg(which, BSP_QDRM_REG_CTRL, BSP_QDRM_RDEN);
    /* wait for rd flag to clear */
    for (j=0; j < 1000; j++){
      if (!(qdr_mem_get_reg(which, BSP_QDRM_REG_CTRL) & BSP_QDRM_RDEN)){
        break;
      }
      if (j == 999) {
        sprintf(bit_strerr, "timeout waiting for read to be completed");
        return -1;
      }
    }
    
    for (j=0; j < BSP_QDRM_DLEN; j++){
      t = qdr_mem_get_reg(which, BSP_QDRM_REG_Q + (j << 2));
#ifdef DEBUG
      printf("%8x[%d]: %08x\n",i,j,t);
#endif
      d = (i % 2) ? (~(i / 2)) : (i/2);
      if (j == 0)
        d &= 0xffff;

      if (t != d){
        sprintf(bit_strerr, "data mismatch: addr = %x, offset=%d, got=%x, expected=%x", i, j, t, d);
        return -1;
      }
    }
  }

  return 0;
}

static int qdr_fabric_test(int which)
{
  sprintf(bit_strerr, "test not implemented");
  return -1;
}

int bit_qdr(int which, int subtest, u32 flags)
{
  switch (subtest){
  case 0:
    return qdr_phy_cal(which, flags);
    break;
  case 1:
    return qdr_mem_test(which, flags);
    break;
  case 2:
    return qdr_fabric_test(which);
    break;
  default:
    sprintf(bit_strerr, "unsupport subtest");
    return -1;
    break;
  }
}

