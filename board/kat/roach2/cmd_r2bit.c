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
#include "include/bit.h"

#ifdef CONFIG_CMD_R2BIT

/* TODO: definition mechanism requires refinement */

/* Extern declaration for function mapping */
extern int bit_tge(int which, int subtest, u32 flags);
extern int bit_v6gbe(int which, int subtest, u32 flags);
extern int bit_v6comm(int which, int subtest, u32 flags);
extern int bit_qdr(int which, int subtest, u32 flags);
extern int bit_zdok(int which, int subtest, u32 flags);

#define BIT_TESTS 5

const char* tge_subtests[2] = {"phy status", "fabric counter test"}; 
const char* v6gbe_subtests[3] = {"phy status", "sgmii status", "ping"}; 
const char* v6comm_subtests[2] = {"version check", "scratchpad access"}; 
const char* zdok_subtests[1] = {"basic connectivity"}; 
const char* qdr_subtests[3] = {"calibration", "ppc access", "fabric"};

static struct bit_mapping bit_list[BIT_TESTS] = {
  {&bit_tge, "tge", 8, 2, tge_subtests},
  {&bit_v6gbe, "v6gbe", 1, 3, v6gbe_subtests},
  {&bit_v6comm, "v6comm", 1, 2, v6comm_subtests},
  {&bit_zdok, "zdok", 2, 1, zdok_subtests},
  {&bit_qdr, "qdr", 4, 3, qdr_subtests},
};

char bit_strerr[256];

void list_bits(void)
{
  int i, j;
  for (i=0; i < BIT_TESTS; i++) {
    printf("%s[%d]", bit_list[i].name, bit_list[i].devices);
    for (j=0; j < bit_list[i].subtests; j++) {
      printf(" (%d: %s)", j, bit_list[i].subtest_name[j]);
    }
    printf("\n");
  }
}

static int run_bit(struct bit_mapping *bitm, int which, int subtest, u32 flags)
{
  int ret;
  ret = (*bitm->test)(which, subtest, flags);
  printf("(test: %s", bitm->name);
  if (bitm->devices != 1)
    printf("[%d]", which);
  printf(", %d: %s) ", subtest, bitm->subtest_name[subtest]);

  if (!ret)
    printf("PASSED\n");
  else
    printf("FAILED: %s\n", bit_strerr);
  return ret;
}

int do_roach2_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  int i, index=-1;
  int ret = 0;
  int arg_which = 0, arg_flags = 0, arg_subtest = -1;

  if (argc < 2) {
    printf("available Built-In Tests (BITs):\n");
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
    if (arg_which < 0 || arg_which > bit_list[index].devices - 1) {
      printf("ERROR: device out of range (max: %d)\n", bit_list[index].devices - 1);
      return -1;
    }
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
      ret |= run_bit(&bit_list[index], arg_which, i, arg_flags);
    }
  } else {
    ret = run_bit(&bit_list[index], arg_which, arg_subtest, arg_flags);
  }

  return ret;
}


U_BOOT_CMD(
	r2bit,	5,	1,	do_roach2_test,
	"run a roach2 built-in tests",
	"<test> [which] [subtest] [flags] - run 'subtest' of 'test' on 'which' device with 'flags'\n"
  "      list - list the availiable tests and subtests"
);
#endif /* CONFIG_CMD_R2BIT */
