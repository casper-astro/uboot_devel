/*
 * (C) Copyright 2006-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,      AMCC/IBM, alain.saurel@fr.ibm.com
 *
 * (C) 2008
 * Marc Welz,               KAT
 *
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

#include <serial.h>
#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/ppc4xx-ebc.h>


#include "include/cpld.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

ulong flash_get_size(ulong base, int banknum);

int board_early_init_f(void)
{
  u32 sdr0_pfc1, sdr0_pfc2;
  u32 reg;

  /*
   * Setup the interrupt controller polarities, triggers, etc.
   */
  mtdcr(UIC0SR, 0xffffffff);  /* clear all */
  mtdcr(UIC0ER, 0x00000000);  /* disable all */
  mtdcr(UIC0CR, 0x00000005);  /* ATI & UIC1 crit are critical */
  mtdcr(UIC0PR, 0xfffff7ff);  /* per ref-board manual */
  mtdcr(UIC0TR, 0x00000000);  /* per ref-board manual */
  mtdcr(UIC0VR, 0x00000000);  /* int31 highest, base=0x000 */
  mtdcr(UIC0SR, 0xffffffff);  /* clear all */

  mtdcr(UIC1SR, 0xffffffff);  /* clear all */
  mtdcr(UIC1ER, 0x00000000);  /* disable all */
  mtdcr(UIC1CR, 0x00000000);  /* all non-critical */
  mtdcr(UIC1PR, 0xffffffff);  /* per ref-board manual */
  mtdcr(UIC1TR, 0x00000000);  /* per ref-board manual */
  mtdcr(UIC1VR, 0x00000000);  /* int31 highest, base=0x000 */
  mtdcr(UIC1SR, 0xffffffff);  /* clear all */

  mtdcr(UIC2SR, 0xffffffff);  /* clear all */
  mtdcr(UIC2ER, 0x00000000);  /* disable all */
  mtdcr(UIC2CR, 0x00000000);  /* all non-critical */
  mtdcr(UIC2PR, 0xffffffff);  /* per ref-board manual */
  mtdcr(UIC2TR, 0x00000000);  /* per ref-board manual */
  mtdcr(UIC2VR, 0x00000000);  /* int31 highest, base=0x000 */
  mtdcr(UIC2SR, 0xffffffff);  /* clear all */

  /* Trace Pins are disabled. SDR0_PFC0 Register */
  mtsdr(SDR0_PFC0, 0x0);
  /* select Ethernet (and optionally IIC1) pins */
  mfsdr(SDR0_PFC1, sdr0_pfc1);
  sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
    SDR0_PFC1_SELECT_CONFIG_4;
#ifdef CONFIG_I2C_MULTI_BUS
  sdr0_pfc1 |= ((sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL);
#endif
  /* Two UARTs, so we need 4-pin mode.  Also, we want CTS/RTS mode. */
  sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_4PINS;
  sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U0ME_CTS_RTS;
  sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U1ME_MASK) | SDR0_PFC1_U1ME_CTS_RTS;

  mfsdr(SDR0_PFC2, sdr0_pfc2);
  sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
    SDR0_PFC2_SELECT_CONFIG_4;
  mtsdr(SDR0_PFC2, sdr0_pfc2);
  mtsdr(SDR0_PFC1, sdr0_pfc1);

  /* PCI arbiter disabled */
  mtsdr(SDR0_PCI0, 0x00000000);

  /* setup NAND FLASH */
  reg = SDR0_CUST0_NDFC_DISABLE | SDR0_CUST0_MUX_GPIO_SEL;
  mtsdr(SDR0_CUST0, reg);

  /* Set EBC to 32-bits - a long shot to fix weird 16-bit issues */
  mtsdr(SDR0_EBC, 0x2000000);

  return 0;
}
 
int misc_init_r(void)
{
#if !defined(CONFIG_SYS_NO_FLASH)
  uint pbcr;
  int size_val = 0;
#endif
  unsigned long usb2d0cr = 0;
  unsigned long usb2phy0cr, usb2h0cr = 0;
  unsigned long sdr0_pfc1;
  u32 reg;
  int major, minor;

  mtdcr(EBC0_CFGADDR, EBC0_CFG);
  mtdcr(EBC0_CFGDATA, EBC_CFG_EBTC_DRIVEN |
                      EBC_CFG_PTD_ENABLE |
                      EBC_CFG_RTC_2048PERCLK |
                      EBC_CFG_EMPH_ENCODE(3) |
                      EBC_CFG_EMPL_ENCODE(3) |
                      EBC_CFG_CSTC_DRIVEN |
                      EBC_CFG_BPR_1DW |
                      EBC_CFG_EMS_MASK |
                      EBC_CFG_PME_DISABLE |
                      EBC_CFG_PMT_ENCODE(0));

/* WARNING: resetting EBC params seems redundant, as similar is already
   done in arch/powerpc/cpu/ppc4xx/cpu_init.c
   TODO: finalize whether we need this...
   */
#if !defined(CONFIG_SYS_NO_FLASH)
  /* Re-do flash sizing to get full correct info */

  /* adjust flash start and offset */

  mtdcr(EBC0_CFGADDR, PB0CR);

  gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
  gd->bd->bi_flashoffset = 0;

  pbcr = mfdcr(EBC0_CFGDATA);
  size_val = ffs(gd->bd->bi_flashsize) - 21;
  pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);

  mtdcr(EBC0_CFGADDR, PB0CR);
  mtdcr(EBC0_CFGDATA, pbcr);

#endif /* CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_SYS_FPGA_BASE
  mtebc(PB1AP, CONFIG_SYS_EBC_PB1AP);
  mtebc(PB1CR, CONFIG_SYS_EBC_PB1CR);
#endif
 
#ifdef CONFIG_SYS_CPLD_BASE
  mtebc(PB2AP, CONFIG_SYS_EBC_PB2AP);
  mtebc(PB2CR, CONFIG_SYS_EBC_PB2CR);
#endif
 
#ifdef CONFIG_SYS_SMAP_BASE
  mtebc(PB3AP, CONFIG_SYS_EBC_PB3AP);
  mtebc(PB3CR, CONFIG_SYS_EBC_PB3CR);
#endif
  /*
   * Re-check to get correct base address
   */
  flash_get_size(gd->bd->bi_flashstart, 0);

	major = *((unsigned char*)(CONFIG_SYS_CPLD_BASE + CPLD_REG_MAJOR));
	minor = *((unsigned char*)(CONFIG_SYS_CPLD_BASE + CPLD_REG_MINOR));


  if (major > 0) { /* if CPLD programmed */
	  printf("CPLD:  %d.%d\n", major, minor);
    int dips;
    /* read dip switches */
	  dips = *((unsigned char*)(CONFIG_SYS_CPLD_BASE + CPLD_REG_DIPS));
    if (!(dips & 0x80)) {
	    printf("UART1: assigning as console default\n");
      if (serial_assign("eserial1")) {
	      printf("warning: failed to assign uart to RS232\n");
      } else {
        serial_init();
        serial_stdio_init();
      }
    }
    /* change bus timing to device paced */
	  debug("ebc: cs2 [cpld]  ap=0x%08x\n", CONFIG_SYS_EBC_PB2AP_ALT);
	  mtebc(PB2AP, CONFIG_SYS_EBC_PB2AP_ALT);
  } else {
	  printf("CPLD:  unprogrammed\n");
  }
 
#ifdef CONFIG_ENV_IS_IN_FLASH
  /* Monitor protection ON by default */
  (void)flash_protect(FLAG_PROTECT_SET,
          -CONFIG_SYS_MONITOR_LEN,
          0xffffffff,
          &flash_info[0]);

  /* Env protection ON by default */
  (void)flash_protect(FLAG_PROTECT_SET,
          CONFIG_ENV_ADDR_REDUND,
          CONFIG_ENV_ADDR_REDUND + 2*CONFIG_ENV_SECT_SIZE - 1,
          &flash_info[0]);
#endif
 
  /*
   * USB stuff: internal phy host only
   */

  /* SDR Setting */
  mfsdr(SDR0_PFC1, sdr0_pfc1);
  mfsdr(SDR0_USB2D0CR, usb2d0cr);
  mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
  mfsdr(SDR0_USB2H0CR, usb2h0cr);

  /* Need to select crystal clock source */
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_CRYSTAL;
  
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
  usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;
  usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
  usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
  usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
  usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;

  /*
   * An 8-bit/60MHz interface is the only possible alternative
   * when connecting the Device to the PHY
   */
  usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
  usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;

  /*
   * To enable the USB 2.0 Device function
   * through the UTMI interface
   */
  usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
  usb2d0cr = usb2d0cr | SDR0_USB2D0CR_EBC_SELECTION;

  sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
  sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

  mtsdr(SDR0_PFC1, sdr0_pfc1);
  mtsdr(SDR0_USB2D0CR, usb2d0cr);
  mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
  mtsdr(SDR0_USB2H0CR, usb2h0cr);

  /* TODO: check reset scheme board/lwmon5/lwmon5.c has another take */

  /*clear resets*/
  udelay (1000);
  mtsdr(SDR0_SRST1, 0x00000000);
  udelay (1000);
  mtsdr(SDR0_SRST0, 0x00000000);

  printf("USB:   Host(int phy)\n");


  mfsdr(SDR0_SRST1, reg);    /* enable security/kasumi engines */
  reg &= ~(SDR0_SRST1_CRYP0 | SDR0_SRST1_KASU0);
  mtsdr(SDR0_SRST1, reg);

  /*
   * Clear PLB4A0_ACR[WRP]
   * This fix will make the MAL burst disabling patch for the Linux
   * EMAC driver obsolete.
   */
  reg = mfdcr(PLB4A0_ACR) & ~PLB4Ax_ACR_WRP_MASK;
  mtdcr(PLB4A0_ACR, reg);

  return 0;
}

int checkboard(void)
{
  char *s = getenv("serial#");

  printf("Board: ROACH2");

  if (s != NULL) {
    puts(", serial# ");
    puts(s);
  }
  putc('\n');

  return (0);
}

#ifdef CONFIG_DRAM_TEST

int testdram(void)
{
  unsigned int i,j, result, val;
  unsigned long *mem = (unsigned long *)0;
  const unsigned long kend = (1024 / sizeof(unsigned long));
  unsigned long k, n, r, st;

  r = ((CONFIG_SYS_MBYTES_SDRAM-10) << 10);
  printf("checking %d MB\n", r >> 10);

  for (k = 0; k < r;
       ++k, mem += (1024 / sizeof(unsigned long))) {
    if ((k & 1023) == 0) {
      printf("%3d MB\r", k / 1024);
    }

    memset(mem, 0xaaaaaaaa, 1024);
    for (n = 0; n < kend; ++n) {
      if (mem[n] != 0xaaaaaaaa) {
        printf("SDRAM test fails at: %08x\n", (uint) & mem[n]);
#if 1
        mfsdram(DDR0_00, st);
        printf("dram: reg 0:   0x%08x\n", st);
        mfsdram(DDR0_01, st);
        printf("dram: reg 1:   0x%08x\n", st);
        printf("dram: content: 0x%08x\n", mem[n]);
#endif
        return 1;
      }
    }

    memset(mem, 0x55555555, 1024);
    for (n = 0; n < kend; ++n) {
      if (mem[n] != 0x55555555) {
        printf("SDRAM test fails at: %08x\n",
               (uint) & mem[n]);
        return 1;
      }
    }

    /* make the processor halt whenever it hits uninit ram, instruction jumps to itself */
    memset(mem, 0x48000000, 1024);
  }
  printf("SDRAM test passes\n");
  return 0;
}
#endif


#ifdef CONFIG_LAST_STAGE_INIT
extern int sensors_config(void);

int last_stage_init()
{


  return sensors_config();
}
#endif
