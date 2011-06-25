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

#include "include/fpga.h"
#include "include/cpld.h"

#ifdef CONFIG_CMD_R2BIT

struct bit_mapping {
  int (*test) (int which, int subtest, u32 flags);
  const char* name;
  uint subtests;
  const char** subtest_name;
};

/* Forward declaration for function mapping */
int bit_v6comm(int which, int subtest, u32 flags);
int bit_qdr(int which, int subtest, u32 flags);
int bit_zdok(int which, int subtest, u32 flags);

#define BIT_TESTS 3

/* TODO: definition mechanism requires refinement */
const char* v6comm_subtests[2] = {"version check", "scratchpad access"}; 
const char* zdok_subtests[1] = {"basic connectivity"}; 
const char* qdr_subtests[3] = {"calibration", "ppc access", "fabric"};

static struct bit_mapping bit_list[BIT_TESTS] = {
  {&bit_v6comm, "v6comm", 2, v6comm_subtests},
  {&bit_zdok, "zdok", 1, zdok_subtests},
  {&bit_qdr, "qdr", 3, qdr_subtests},
};

static char bit_strerr[256];

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

/***************************** QDR  *******************************/


/******* CAL ********/

void qdr_set_reg(int which, u32 reg, u32 val)
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

u32 qdr_get_reg(int which, u32 reg)
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

void qdr_dll_reset(int which)
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
int qdr_phy_tick_dly(int which, int dir)
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

#define EDGE_CLEARANCE 9

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

int qdr_align_bit(int which, int bit)
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
    bit_line[i] = is_stable ? val : 0xb;

    if (is_stable && (val == 1 || val == 2)){
      if (!first_val){
        first_val = val;
      }
      stable_count++;

      if (val != prev_val)
        stable_count = 0;
         
      if (first_val != val && stable_count >= STABLE_THRESHOLD){
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
    prev_val = val;

    qdr_phy_tick_dly(which, 1);
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
  }
  return ret;
}

void qdr_dump_bit(int which, int bit)
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

int qdr_phy_cal(int which, u32 flags)
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
    if (flags & 0x1)
      qdr_dump_bit(which, i);
    ret = qdr_align_bit(which, i);
    if (ret)
      break;
  }

  /* disable calibration logic */
  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL, BSP_QDRCONF_DLL_RUN);

  return ret;
}

/******* MEM ********/

void qdr_mem_set_reg(int which, u32 reg, u32 val)
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

u32 qdr_mem_get_reg(int which, u32 reg)
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

int qdr_mem_test(int which)
{
  u32 i, j;
  u32 d, t;
  d = qdr_mem_get_reg(which, BSP_QDRM_REG_STAT);

  if (!(d & BSP_QDRM_PHYRDY)) {
    sprintf(bit_strerr, "qdr phy not ready");
    return -1;
  }

  if ((d & BSP_QDRM_CALFAIL)) {
    sprintf(bit_strerr, "qdr phy calibration failed");
    return -1;
  }

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

int qdr_fabric_test(int which)
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
    return qdr_mem_test(which);
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

void list_bits(void)
{
  int i, j;
  for (i=0; i < BIT_TESTS; i++) {
    printf("%s", bit_list[i].name);
    for (j=0; j < bit_list[i].subtests; j++) {
      printf(" (%d: %s)", j, bit_list[i].subtest_name[j]);
    }
    printf("\n");
  }
}

int do_roach2_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  int i, index=-1;
  int ret, ret_acc = 0;
  int arg_which = 0, arg_flags = 0, arg_subtest = -1;

  if (argc < 2)
    return cmd_usage(cmdtp);

  /* first check for the list command */
  if (!strcmp(argv[1], "list")){
    list_bits();
    return 0;
  }

  for (i = 0; i < BIT_TESTS; i++) {
    if (!strcmp(argv[1], bit_list[i].name)){
      index = i;
    }
  }

  if (index < 0) {
    printf("ERROR: bit %s is not a supported built-in test\n", argv[1]);
    return -1;
  }

  if (argc > 2) {
    arg_which = simple_strtoul(argv[2], NULL, 16);
  }

  if (argc > 3) {
    arg_subtest = simple_strtoul(argv[3], NULL, 16);
    if (arg_subtest > bit_list[index].subtests - 1) {
      printf("ERROR: invalid subtest specified\n");
      return -1;
    }
  }

  if (argc > 4) {
    arg_flags = simple_strtoul(argv[4], NULL, 16);
  }

  if (arg_subtest < 0) {
    for (i=0; i < bit_list[index].subtests; i++) {
      ret = (*bit_list[index].test)(arg_which, i, arg_flags);
      printf("(test: %s, subtest[%d]: %s) ", bit_list[index].name, i, bit_list[index].subtest_name[i]);
      if (!ret)
        printf("PASSED\n");
      else
        printf("FAILED: %s\n", bit_strerr);
      ret_acc |= ret;
    }
  } else {
    ret_acc = (*bit_list[index].test)(arg_which, arg_subtest, arg_flags);
    printf("(test: %s, subtest[%d]: %s) ", bit_list[index].name, arg_subtest, bit_list[index].subtest_name[arg_subtest]);
    if (!ret_acc)
      printf("PASSED\n");
    else
      printf("FAILED: %s\n", bit_strerr);
  }

  return ret_acc;
}


U_BOOT_CMD(
	r2bit,	5,	1,	do_roach2_test,
	"run a roach2 built-in tests",
	"<test> [which] [subtest] [flags] - run 'subtest' of 'test' on 'which' device with 'flags'\n"
  "      list - list the availiable tests and subtests"
);
#endif /* CONFIG_CMD_R2BIT */
