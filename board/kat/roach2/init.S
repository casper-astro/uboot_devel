/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm-offsets.h>
#include <ppc_asm.tmpl>
#include <asm/mmu.h>
#include <config.h>

/*
 * TLB TABLE
 *
 * This table is used by the cpu boot code to setup the initial tlb
 * entries. Rather than make broad assumptions in the cpu source tree,
 * this table lets each board set things up however they like.
 *
 *  Pointer to the table is returned in r1
 */
    .section .bootpg,"ax"
    .globl tlbtab

tlbtab:
	tlbtab_start

#if 0
	/* vxWorks needs this as first entry for the Machine Check interrupt */
	tlbentry( 0x40000000, SZ_256M, 0, 0, AC_RWX | SA_IG )
#endif

	tlbentry( CONFIG_SYS_BOOT_BASE_ADDR, SZ_256M, CONFIG_SYS_BOOT_BASE_ADDR, 1, AC_RWX | SA_G )

  /* SDRAM entries */
	tlbentry( CONFIG_SYS_SDRAM_BASE,  SZ_256M, CONFIG_SYS_SDRAM_BASE,  0, AC_RWX | SA_IG )
#ifdef CONFIG_SYS_SDRAM_BASE1
	tlbentry( CONFIG_SYS_SDRAM_BASE1, SZ_256M, CONFIG_SYS_SDRAM_BASE1, 0, AC_RWX | SA_IG )
#endif

	/* TLB-entry for Internal Registers & OCM */
	tlbentry( 0xe0000000, SZ_16M, 0xe0000000, 0,  AC_RWX | SA_I )

	/* TLB-entry for peripherals */
	tlbentry( 0xEF000000, SZ_16M, 0xEF000000, 1, AC_RWX | SA_IG)

#ifdef CONFIG_SYS_CPLD_BASE
	/* TLB-entry for cpld on EBC */
	tlbentry( CONFIG_SYS_CPLD_BASE, SZ_1M, CONFIG_SYS_CPLD_BASE, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
#endif

#ifdef CONFIG_SYS_SMAP_BASE
	/* TLB-entry for selectmap on EBC */
	tlbentry( CONFIG_SYS_SMAP_BASE, SZ_1M, CONFIG_SYS_SMAP_BASE, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
#endif

#ifdef CONFIG_SYS_FPGA_BASE
	/* TLB-entry for FPGA */
	tlbentry( CONFIG_SYS_FPGA_BASE, SZ_256M, CONFIG_SYS_FPGA_BASE, 1, AC_R|AC_W|AC_X|SA_G|SA_I )
#endif

	tlbtab_end

