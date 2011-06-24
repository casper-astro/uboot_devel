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

#ifdef CONFIG_CMD_R2DEBUG
static int do_roach2_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  u32 val;

  mfebc(EBC0_CFG,val);
  printf("ebc:            cfg=0x%08x\n", val);

  mfebc(PB0AP,val);
  printf("ebc: cs0 [flash] ap=0x%08x\n", val);
  mfebc(PB0CR,val);
  printf("ebc: cs0 [flash] cr=0x%08x\n", val);
  mfebc(PB1AP,val);
  printf("ebc: cs1 [fpga]  ap=0x%08x\n", val);
  mfebc(PB1CR,val);
  printf("ebc: cs1 [fpga]  cr=0x%08x\n", val);

  mfebc(PB2AP,val);
  printf("ebc: cs2 [cpld]  ap=0x%08x\n", val);
  mfebc(PB2CR,val);
  printf("ebc: cs2 [cpld]  cr=0x%08x\n", val);
  mfebc(PB3AP,val);
  printf("ebc: cs3 [smap]  ap=0x%08x\n", val);
  mfebc(PB3CR,val);
  printf("ebc: cs3 [smap]  cr=0x%08x\n", val);
  mfebc(PBEAR,val);
  printf("ebc:            ear=0x%08x\n", val);
  mfebc(PBESR0,val);
  printf("ebc:           esr0=0x%08x\n", val);
  mfebc(PBESR1,val);
  printf("ebc:           esr1=0x%08x\n", val);

  printf("GPIO0_OR           :   0x%08lx\n", in32(GPIO0_OR     ));
  printf("GPIO0_TCR          :   0x%08lx\n", in32(GPIO0_TCR    ));
  printf("GPIO0_OSRL         :   0x%08lx\n", in32(GPIO0_OSRL   ));
  printf("GPIO0_OSRH         :   0x%08lx\n", in32(GPIO0_OSRH   ));
  printf("GPIO0_TSRL         :   0x%08lx\n", in32(GPIO0_TSRL   ));
  printf("GPIO0_TSRH         :   0x%08lx\n", in32(GPIO0_TSRH   ));
  printf("GPIO0_ISR1L        :   0x%08lx\n", in32(GPIO0_ISR1L  ));
  printf("GPIO0_ISR1H        :   0x%08lx\n", in32(GPIO0_ISR1H  ));
  printf("GPIO0_ISR2L        :   0x%08lx\n", in32(GPIO0_ISR2L  ));
  printf("GPIO0_ISR2H        :   0x%08lx\n", in32(GPIO0_ISR2H  ));
  printf("GPIO0_ISR3L        :   0x%08lx\n", in32(GPIO0_ISR3L  ));
  printf("GPIO0_ISR3H        :   0x%08lx\n", in32(GPIO0_ISR3H  ));
                       
  printf("GPIO1_OR           :   0x%08lx\n", in32(GPIO1_OR     ));
  printf("GPIO1_TCR          :   0x%08lx\n", in32(GPIO1_TCR    ));
  printf("GPIO1_OSRL         :   0x%08lx\n", in32(GPIO1_OSRL   ));
  printf("GPIO1_OSRH         :   0x%08lx\n", in32(GPIO1_OSRH   ));
  printf("GPIO1_TSRL         :   0x%08lx\n", in32(GPIO1_TSRL   ));
  printf("GPIO1_TSRH         :   0x%08lx\n", in32(GPIO1_TSRH   ));
  printf("GPIO1_ISR1L        :   0x%08lx\n", in32(GPIO1_ISR1L  ));
  printf("GPIO1_ISR1H        :   0x%08lx\n", in32(GPIO1_ISR1H  ));
  printf("GPIO1_ISR2L        :   0x%08lx\n", in32(GPIO1_ISR2L  ));
  printf("GPIO1_ISR2H        :   0x%08lx\n", in32(GPIO1_ISR2H  ));
  printf("GPIO1_ISR3L        :   0x%08lx\n", in32(GPIO1_ISR3L  ));
  printf("GPIO1_ISR3H        :   0x%08lx\n", in32(GPIO1_ISR3H  ));


  return 0;
}

U_BOOT_CMD(
	r2debug,	3,	1,	do_roach2_debug,
	"print some useful debug info",
	"[level] - optional debug level"
);
#endif /* CONFIG_CMD_R2DEBUG */
