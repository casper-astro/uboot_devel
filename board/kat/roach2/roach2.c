/*
 * (C) Copyright 2006-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,	    AMCC/IBM, alain.saurel@fr.ibm.com
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

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/ppc4xx.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

ulong flash_get_size(ulong base, int banknum);

int board_early_init_f(void)
{
	u32 sdr0_pfc1, sdr0_pfc2;

	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	mtdcr(EBC0_CFGDATA, 0xb8400000);

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffff7ff);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC2TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

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
	char *act = getenv("usbact");
	u32 reg;

#if !defined(CONFIG_SYS_NO_FLASH)
	/* Re-do flash sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(gd->bd->bi_flashsize) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtdcr(EBC0_CFGDATA, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

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
#endif /* CONFIG_SYS_NO_FLASH */

	/*
	 * USB stuff...
	 */
	if (act == NULL || strcmp(act, "hostdev") == 0) {
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
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_USB2DEV_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_USB2D_SEL;

		mtsdr(SDR0_PFC1, sdr0_pfc1);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2H0CR, usb2h0cr);

		/*clear resets*/
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Host(int phy) Device(ext phy)\n");

	} else if (strcmp(act, "dev") == 0) {
		/*-------------------PATCH-------------------------------*/
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x672c6000);

		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000080);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60206000);

		*(unsigned int *)(0xe0000350) = 0x00000001;

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60306000);
		/*-------------------PATCH-------------------------------*/

		/* SDR Setting */
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);
		mfsdr(SDR0_USB2D0CR, usb2d0cr);
		mfsdr(SDR0_PFC1, sdr0_pfc1);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PUREN;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_DEV;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_DEV;

		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_8BIT_60MHZ;

		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_EBC_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

		mtsdr(SDR0_USB2H0CR, usb2h0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_PFC1, sdr0_pfc1);

		/* clear resets */
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Device(int phy)\n");
	}

	mfsdr(SDR0_SRST1, reg);		/* enable security/kasumi engines */
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

	printf("Board: ROACH2 - CASPER/KAT reconfigurable computing board");

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

  /*
  #define MEM_SIZE 0x1fffffff
  for (j =0; j < 1; j++) {
    printf("Writing ramp into whole 512MB memory\n");
    for (i = 0; i < (MEM_SIZE >> 4); i++){
      mem[i] = i;
    }

    printf("Verify ramp\n");
    result = 1;
    for (i = 0; i < (MEM_SIZE >> 4); i++){
      val = mem[i];
      if (val != i){
        result = 0;
        printf("Memory mismatch at address %x, got %x, expected %x\n",i<<4,val, i);
        break;
      }
      if (i%1024 == 1023) {
        printf(".");
      }
      if (i%(1024*16) == 1024*16-1) {
        printf("x\n");
      }
      if (i%(1024*1024) == 1024*1024-1) {
        printf("%d of %d\n", i, MEM_SIZE >> 4);
      }
    }
    if (result) {
      printf("ramp ok\n");
    } else {
      printf("ramp bad\n");
    }
  }
  */


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
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
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
