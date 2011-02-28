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

  return ret;
}

/*********** QDR calibration *************/

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
      printf("error: qdr data sampled done stuck low\n");
      ret = 1;
    }
  }
  return ret;
}

#define STABLE_THRESHOLD 4

#define EDGE_CLEARANCE 9

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

  int cal_pos = EDGE_CLEARANCE;

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
        if (i + EDGE_CLEARANCE - STABLE_THRESHOLD >= 31){
          cal_pos = i - baddies - STABLE_THRESHOLD - EDGE_CLEARANCE;
          if (cal_pos < EDGE_CLEARANCE){
            printf("warning: bit %d calibation point %d may not be safe\n", bit, cal_pos);
          }
          break;
        } else {
          cal_pos = i + EDGE_CLEARANCE - STABLE_THRESHOLD;
          if (i >= 32){
            ret = 1;
            printf("error: bit %d calibration point %d over the edge\n", bit, cal_pos);
            cal_pos = EDGE_CLEARANCE;
          }
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
    if (i==31){
      printf("warning: no edge detected, using default cal_pos\n");
      printf("b = %d, g = %d, f = %d\n", baddies, stable_count, first_val);
      baddies = 100;
    }
  }
  if (baddies == 100){
    for (i=0;i < 32; i++){
      printf("%x",bit_line[i]);
    }
    printf("\n");
  }
#ifdef DEBUG
  printf(" cal pos = %d, ", cal_pos);
#endif
  qdr_set_reg(which, BSP_QDRCONF_REG_BITCTRL, BSP_QDRCONF_DLY_RST);
  udelay(10);
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
#ifdef DEBUG
    printf("align used\n");
#endif
  } else {
#ifdef DEBUG
    printf("align bypass\n");
#endif
  }
  if (!is_stable || val == 0 || val == 3){
    printf("error: apparent error in calibration\n");
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
  //printf("\n");
}

int qdr_phy_cal(int which)
{
  int i;
  int ret=0;
  u32 dreg;

  qdr_dll_reset(which);

  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL, BSP_QDRCONF_CAL_EN | BSP_QDRCONF_DLL_RUN);

  for (i=0; i < 1000; i++){
    dreg = qdr_get_reg(which, BSP_QDRCONF_REG_STATUS);
    if (dreg & BSP_QDRCONF_CAL_RDY)
      break;
      
    if (i==999){
      printf("error: cal state machine not ready\n");
    }
  }

  for (i=0; i < 36; i++) {
#ifdef DEBUG
    qdr_dump_bit(which, i);
#endif
    ret = ret | qdr_align_bit(which, i);
  }

  qdr_set_reg(which, BSP_QDRCONF_REG_CTRL, BSP_QDRCONF_DLL_RUN);

  return ret;
}

/*********** QDR memory testing *************/

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
  int ret=0;
  u32 i, j;
  u32 d, t;
  d = qdr_mem_get_reg(which, BSP_QDRM_REG_STAT);

  if (!(d & BSP_QDRM_PHYRDY))
    printf("warning: qdr phy not ready for mem test\n");

  if ((d & BSP_QDRM_CALFAIL))
    printf("warning: qdr mem test with cal failed\n");

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
         printf("warning: timeout waiting for qdr read completion\n");
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
        ret = 1;
        printf("FAIL: QDR%d, data mismatch: addr = %x, offset=%d, got=%x, expected=%x\n", which, i, j, t, d);
      }
    }
    if (ret)
      break;
  }
  if (!ret){
    printf("PASSED: QDR%d memory test\n", which);
  }

  return ret;
}

int qdr_fabric_test(int which)
{
  printf("warning: QDR fabric test not implemented\n");
  return 0;
}

int bit_qdr(int which)
{
  int ret = 0;
  if ((ret |= qdr_phy_cal(which))) {
    printf("error: QDR-%d calibration failed\n", which);
  }

  if ((ret |= qdr_mem_test(which))) {
    printf("error: QDR-%d cpu memory test failed\n", which);
  }

  if ((ret |= qdr_fabric_test(which))) {
    printf("error: QDR-%d fabric test failed\n", which);
  }

  return ret;
}


static int do_roach2_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  /*
  if (bit_zdok(0)) {
    printf("FAIL: ZDOK0 test failed\n");
  } else {
    printf("PASS: ZDOK1 test passed\n");
  }
  if (bit_zdok(1)) {
    printf("FAIL: ZDOK0 test failed\n");
  } else {
    printf("PASS: ZDOK1 test passed\n");
  }
  */

  bit_qdr(0);
  bit_qdr(1);
  bit_qdr(2);
  bit_qdr(3);

  return 0;
}


U_BOOT_CMD(
	roach2_test,	3,	1,	do_roach2_test,
	"roach2_test - run through roach-2 built-in tests\n",
	"" ""
);
#endif /* CONFIG_CMD_ROACH2_TEST */
