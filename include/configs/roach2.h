/*
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 *
 * (C) Copyright 2008
 * Marc Welz,               KAT
 *
 * (C) Copyright 2011
 * David George,            KAT
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

/*-----------------------------------------------------------------------
 * roach2.h - configuration for roach2 board
 *----------------------------------------------------------------------*/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/

#define CONFIG_ROACH2     1    /* Board is ROACH 2 */
#define CONFIG_HOSTNAME   roach2
#define CONFIG_440EPX     1  /* Specific PPC440EPx    */
#define CONFIG_440        1  /* ... PPC440 family    */
#define CONFIG_4xx        1  /* ... PPC4xx family    */



/* ROACH 2 has a sysclk fixed at 33.3333 MHz */
#define CONFIG_SYS_CLK_FREQ 33333333

#define CONFIG_BOARD_EARLY_INIT_F 1  /* Call board_early_init_f  */
#define CONFIG_MISC_INIT_R        1  /* Call misc_init_r    */
#define CONFIG_LAST_STAGE_INIT    1  /* call last_stage_init */

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the actual
 * resources get mapped (not physical addresses).
 *----------------------------------------------------------------------*/

#define CONFIG_SYS_TEXT_BASE  0xFFF80000

#define CONFIG_SYS_SDRAM_BASE   0x00000000  /* _must_ be 0    */
#define CONFIG_SYS_SDRAM_BASE1  0x10000000
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE  /* Start of U-Boot  */
#define CONFIG_SYS_MONITOR_LEN  (0xFFFFFFFF - CONFIG_SYS_MONITOR_BASE + 1)
#define CONFIG_SYS_MALLOC_LEN   (1 << 20)  /* Reserved for malloc  */


#define CONFIG_SYS_BOOT_BASE_ADDR     0xf0000000
#define CONFIG_SYS_FLASH_BASE         0xf8000000  /* start of FLASH  */

#define CONFIG_SYS_PCI_BASE		    0xe0000000	/* Internal PCI regs	*/
#define CONFIG_SYS_OCM_BASE      0xe0010000  /* On-chip memory */
#define CONFIG_SYS_OCM_DATA_ADDR CONFIG_SYS_OCM_BASE

#define CONFIG_SYS_USB2D0_BASE    0xe0000100
#define CONFIG_SYS_USB_DEVICE     0xe0000000
#define CONFIG_SYS_USB_HOST       0xe0000400


/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/

/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache  */
#define CONFIG_SYS_INIT_RAM_ADDR   CONFIG_SYS_OCM_BASE  /* OCM      */
#define CONFIG_SYS_INIT_RAM_SIZE   (4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET  (CONFIG_SYS_GBL_DATA_OFFSET - 0x4)


/*-----------------------------------------------------------------------
 * Serial Ports : ROACH 2 has an FTDI serial on UART0 and RS232 on UART1
 *----------------------------------------------------------------------*/

#define CONFIG_SERIAL_MULTI
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE  1
#define CONFIG_SYS_NS16550_CLK    get_serial_clock()
#define CONFIG_BAUDRATE    115200
#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}
/*#define CONFIG_CONS_INDEX  (2) */      /* UART1 (RS232) */

#define CONFIG_CONS_INDEX  (1)      /* UART0 (FTDI)  */

#undef CONFIG_SYS_EXT_SERIAL_CLOCK /* No external UART clk on ROACH 2 */ 

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/

#define CONFIG_HARD_I2C      /* I2C with hardware support  */
#define CONFIG_PPC4XX_I2C    /* use PPC4xx driver    */
#define CONFIG_SYS_I2C_SLAVE    0x7F
/*#define CONFIG_SYS_I2C_SPEED    400000 */ /* I2C speed and slave address  */
#define CONFIG_SYS_I2C_SPEED    100000  /* I2C speed and slave address  */

/* I2C bootstrap EEPROM */
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR   0x54 /* Boot Option G (0xA8 >> 1) */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET 0
#define CONFIG_4xx_CONFIG_BLOCKSIZE         16

/* I2C SYSMON (LM75, AD7414 is almost compatible)      */
#define CONFIG_DTT_LM75            1  /* use LM75 (AD7414 is treated as subset) */
#define CONFIG_DTT_AD7414          1  /* use AD7414 work arounds */
#define CONFIG_SYS_I2C_DTT_ADDR    0x4c /* Air outlet temperature */
#define CONFIG_DTT_SENSORS         {0x0, 0x2}  /* Sensor address offsets for dtt command*/
#define CONFIG_SYS_DTT_MAX_TEMP    70
#define CONFIG_SYS_DTT_LOW_TEMP    -30
#define CONFIG_SYS_DTT_HYSTERESIS  3

/*-----------------------------------------------------------------------
 * DDR2 SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MBYTES_SDRAM  (512)     /* 512 MB */
#if 0
#define CONFIG_DDR_DATA_EYE                /* use DDR2 optimization  */
#define CONFIG_DRAM_TEST
#endif
#define CONFIG_SYS_MEM_TOP_HIDE  (4 << 10) /* don't use last 4kbytes  */
          /* 440EPx errata CHIP 11  */

/*-----------------------------------------------------------------------
 * Ethernet/EMAC/PHY
 *----------------------------------------------------------------------*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_M88E1111_PHY  1
#define CONFIG_PHY_ADDR      0x1

#define CONFIG_PHY_RESET     1 /* reset phy upon startup  */
#define CONFIG_PHY_GIGE      1 /* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0
#define CONFIG_IBM_EMAC4_V4  1

#define CONFIG_MII             /* MII PHY management    */
#define CONFIG_NET_MULTI      
/* We only have one ethernet port so the above should be undefined,
   but for some reason that breaks the compile.  */

#define CONFIG_SYS_RX_ETH_BUFFER  32  /* number of eth rx buffers  */

/*-----------------------------------------------------------------------
 * USB
 *----------------------------------------------------------------------*/

#undef CONFIG_USB_EHCI  /* OHCI by default */
/* The PPC440EPx has a somewhat flakey EHCI controller so we would
   prefer to use the OHCI by default */

#ifdef CONFIG_USB_EHCI

#define CONFIG_USB_EHCI_PPC4XX
#define CONFIG_SYS_PPC4XX_USB_ADDR  0xe0000300
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_EHCI_DESC_BIG_ENDIAN

#ifdef CONFIG_4xx_DCACHE
#define CONFIG_EHCI_DCACHE
#endif

#else /* CONFIG_USB_EHCI */

#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER

#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT  1
#define CONFIG_SYS_USB_OHCI_REGS_BASE  CONFIG_SYS_USB_HOST
#define CONFIG_SYS_USB_OHCI_SLOT_NAME  "ppc440"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 15

#endif

#define CONFIG_USB_STORAGE
/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/

#define CONFIG_SYS_FLASH_CFI      /* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER    /* Use common CFI driver  */

#define CONFIG_SYS_FLASH_BANKS_LIST  { CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_MAX_FLASH_BANKS  1  /* max number of memory banks        */
#define CONFIG_SYS_MAX_FLASH_SECT  1024  /* max number of sectors on one chip (1Gb)  */

#define CONFIG_SYS_FLASH_ERASE_TOUT  120000  /* Timeout for Flash Erase (in ms)    */
#define CONFIG_SYS_FLASH_WRITE_TOUT  500  /* Timeout for Flash Write (in ms)    */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1  /* use buffered writes (20x faster)   */
#define CONFIG_SYS_FLASH_PROTECTION  1  /* use hardware flash protection      */

#define CONFIG_SYS_FLASH_EMPTY_INFO        /* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST  1  /* don't warn upon unknown flash      */

#define CONFIG_ENV_IS_IN_FLASH 1
#define CONFIG_ENV_OVERWRITE 1 /* allows eth addr to be set */

#define CONFIG_ENV_SECT_SIZE  0x20000  /* size of one complete sector        */
#define CONFIG_ENV_ADDR    ((-CONFIG_SYS_MONITOR_LEN)-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE    0x2000  /* Total Size of Environment Sector   */

/* Address and size of Redundant Environment Sector  */
#define CONFIG_ENV_ADDR_REDUND  (CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND  (CONFIG_ENV_SIZE)

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Commands
 *---------------------------------------------------------------------*/

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_USB

#define CONFIG_CMD_R2SMAP
#define CONFIG_CMD_R2DEBUG
#define CONFIG_CMD_R2GPIO
#define CONFIG_CMD_R2SENSORS
#define CONFIG_CMD_R2BIT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/

#define CONFIG_BOOTDELAY     5      /* autoboot after 5 seconds  */
#define CONFIG_SYS_LONGHELP         /* undef to save memory    */
#define CONFIG_SYS_PROMPT    "=> "  /* Monitor Command Prompt  */
#define CONFIG_SYS_CBSIZE    1024   /* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE    (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS   16  /* max number of command args  */
#define CONFIG_SYS_BARGSIZE  CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size  */

#define CONFIG_SYS_MEMTEST_START  0x0400000 /* memtest works on    */
#define CONFIG_SYS_MEMTEST_END    0x0C00000 /* 4 ... 12 MB in DRAM  */

#define CONFIG_SYS_LOAD_ADDR      0x100000  /* default load address  */
#define CONFIG_SYS_EXTBDINFO      /* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ      1000  /* decrementer freq: 1 ms ticks  */

#define CONFIG_CMDLINE_EDITING        /* add command line history  */
#define CONFIG_AUTO_COMPLETE          /* add autocompletion support  */
#define CONFIG_LOOPW                  /* enable loopw command         */
#define CONFIG_MX_CYCLIC              /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK   /* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE       /* include version env variable */

#define CONFIG_SYS_HUSH_PARSER        /* Use the HUSH parser    */
#ifdef  CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2  "> "
#endif

#define CONFIG_LOADS_ECHO               /* echo on for serial download  */
#define CONFIG_SYS_LOADS_BAUD_CHANGE    /* allow baudrate change  */

/*-----------------------------------------------------------------------
 * BOOTP options
 *----------------------------------------------------------------------*/
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the 40x Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ    (16 << 20) /* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTM_LEN    (16 << 20) /* Increase max gunzip size */

/*
 * Pass open firmware flat tree
 */

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
/* Update size in "reg" property of NOR FLASH device tree nodes */
#define CONFIG_FDT_FIXUP_NOR_FLASH_SIZE

/*-----------------------------------------------------------------------
 * Booting and default environment
 *----------------------------------------------------------------------*/

#define CONFIG_PREBOOT         "echo;" \
  "echo CONFIG_PREBOOT needs to be defined;" \
  "echo"
#define CONFIG_BOOTCOMMAND  "run flash_self"

#define CONFIG_ROOTPATH "rootpath=/opt/eldk/ppc_4xxFP\0"
#define CONFIG_BOOTFILE    "bootfile=/tftpboot/roach/uImage\0"

#define CONFIG_EXTRA_ENV_SETTINGS     \
        CONFIG_BOOTFILE \
        CONFIG_ROOTPATH \
        "netdev=eth0\0" \
        "ramargs=setenv bootargs root=/dev/ram rw\0" \
        "addip=setenv bootargs ${bootargs} " \
         "ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}" \
         ":${hostname}:${netdev}:off panic=1\0" \
        "addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
        "flash_self=run ramargs addip addtty;" \
         "bootm ${kernel_addr} ${ramdisk_addr}\0" \
        "ethaddr=02:02:02:02:02:00\0" \
        "kernel_addr=FC000000\0" \
        "ramdisk_addr=FC180000\0" \
        "clearenv=protect off FFF40000 FFF7FFFF;era FFF40000 FFF7FFFF\0" \
        "update=protect off FFF80000 FFFFFFFF;era FFF80000 FFFFFFFF;dhcp;tftp 200000 u-boot.bin;cp.b 200000 FFF80000 80000\0" \
        ""


/*-----------------------------------------------------------------------
 * POST support
 *----------------------------------------------------------------------*/

#define CONFIG_SYS_POST_FPU_ON    CONFIG_SYS_POST_FPU

#define CONFIG_SYS_POST_MEMORY_ON  CONFIG_SYS_POST_MEMORY

#define CONFIG_POST    (CONFIG_SYS_POST_CACHE     | \
         CONFIG_SYS_POST_CPU     | \
         CONFIG_SYS_POST_ETHER     | \
         CONFIG_SYS_POST_FPU_ON    | \
         CONFIG_SYS_POST_I2C     | \
         CONFIG_SYS_POST_MEMORY_ON | \
         CONFIG_SYS_POST_SPR     | \
         CONFIG_SYS_POST_UART)

#define CONFIG_LOGBUFFER
#define CONFIG_SYS_POST_CACHE_ADDR  0x7fff0000  /* free virtual address     */

#define CONFIG_SYS_CONSOLE_IS_IN_ENV  /* Otherwise it catches logbuffer as output */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/* supporting AP macros */
#define EBC_AP_BME     0x80000000
#define EBC_AP_TWT(p)  (((p) << 23) & 0x7f800000)
#define EBC_AP_CSN(p)  (((p) << 18) & 0x000c0000)
#define EBC_AP_OEN(p)  (((p) << 16) & 0x00030000)
#define EBC_AP_WBN(p)  (((p) << 14) & 0x0000c000)
#define EBC_AP_WBF(p)  (((p) << 12) & 0x00003000)
#define EBC_AP_TH(p)   (((p) <<  9) & 0x00000e00)
#define EBC_AP_RE      0x00000100
#define EBC_AP_SOR     0x00000080
#define EBC_AP_BEM     0x00000040
#define EBC_AP_PEN     0x00000020
/* supporting bank macros */
#define EBC_BANK_8B    0x00000000
#define EBC_BANK_16B   0x00002000
#define EBC_BANK_32B   0x00004000
#define EBC_BANK_READ  0x00008000
#define EBC_BANK_WRITE 0x00010000
#define EBC_BANK_1M    0x00000000
#define EBC_BANK_2M    0x00020000
#define EBC_BANK_4M    0x00040000
#define EBC_BANK_8M    0x00060000
#define EBC_BANK_16M   0x00080000
#define EBC_BANK_32M   0x000a0000
#define EBC_BANK_64M   0x000c0000
#define EBC_BANK_128M  0x000e0000


#define CONFIG_FLASH_BASE CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_CPLD_BASE   0xC0000000
#define CONFIG_SYS_FPGA_BASE   0xD0000000
#define CONFIG_SYS_SMAP_BASE   0xC0100000

#define CONFIG_SYS_EBC_CFG 0xbe4c0000

/* Bank 0 (NOR-FLASH) initialization */
#define CONFIG_SYS_EBC_PB0AP  (EBC_AP_TWT(13) | EBC_AP_CSN(0) | EBC_AP_OEN(2) | EBC_AP_WBN(1) | EBC_AP_WBF(3) | EBC_AP_TH(2))
#define CONFIG_SYS_EBC_PB0CR  (CONFIG_SYS_FLASH_BASE | EBC_BANK_128M | EBC_BANK_READ | EBC_BANK_WRITE | EBC_BANK_16B)

/* Bank 1 (FPGA) initialization */
#ifdef CONFIG_SYS_FPGA_BASE
#define CONFIG_SYS_EBC_PB1AP  (EBC_AP_TWT(1) | EBC_AP_CSN(0) | EBC_AP_OEN(0) | EBC_AP_WBN(0) | EBC_AP_WBF(0) | EBC_AP_TH(1) | EBC_AP_RE | EBC_AP_SOR)
#define CONFIG_SYS_EBC_PB1CR  (CONFIG_SYS_FPGA_BASE | EBC_BANK_128M | EBC_BANK_READ | EBC_BANK_WRITE | EBC_BANK_32B)
#endif

/* Bank 2 (CPLD) initialization */
#ifdef CONFIG_SYS_CPLD_BASE
#define CONFIG_SYS_EBC_PB2AP  (EBC_AP_TWT(1) | EBC_AP_CSN(0) | EBC_AP_OEN(0) | EBC_AP_WBN(0) | EBC_AP_WBF(0) | EBC_AP_TH(1))
#define CONFIG_SYS_EBC_PB2CR  (CONFIG_SYS_CPLD_BASE | EBC_BANK_1M | EBC_BANK_READ | EBC_BANK_WRITE | EBC_BANK_8B)

#define CONFIG_SYS_EBC_PB2AP_ALT (EBC_AP_TWT(2) | EBC_AP_CSN(0) | EBC_AP_OEN(0) | EBC_AP_WBN(0) | EBC_AP_WBF(0) | EBC_AP_TH(1) | EBC_AP_RE)
#define CONFIG_SYS_EBC_PB2CR_ALT (CONFIG_SYS_CPLD_BASE | EBC_BANK_1M | EBC_BANK_READ | EBC_BANK_WRITE | EBC_BANK_8B)
#endif

/* Bank 3 (selectmap) initialization */
#ifdef CONFIG_SYS_SMAP_BASE
#define CONFIG_SYS_EBC_PB3AP  (EBC_AP_TWT(0) | EBC_AP_CSN(0) | EBC_AP_OEN(0) | EBC_AP_WBN(0) | EBC_AP_WBF(0) | EBC_AP_TH(0))
#define CONFIG_SYS_EBC_PB3CR  (CONFIG_SYS_SMAP_BASE | EBC_BANK_1M  | EBC_BANK_READ | EBC_BANK_WRITE | EBC_BANK_32B)
#endif

/* TODO: need to add second FPGA chip select for 'fast' interface */

/*
 * PPC440 GPIO Configuration
 */
/* test-only: take GPIO init from pcs440ep ???? in config file */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*    Out      GPIO  Alternate1  Alternate2  Alternate3 */ \
{ \
/* GPIO Core 0 */ \
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO0  EBC_ADDR(7)  DMA_REQ(2)  */  \
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO1  EBC_ADDR(6)  DMA_ACK(2)  */  \
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO2  EBC_ADDR(5)  DMA_EOT/TC(2)  */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO3  EBC_ADDR(4)  DMA_REQ(3)  */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO4  EBC_ADDR(3)  DMA_ACK(3)  */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO5  EBC_ADDR(2)  DMA_EOT/TC(3)  */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO6  EBC_CS_N(1)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO7  EBC_CS_N(2)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO8  EBC_CS_N(3)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO9  EBC_CS_N(4)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_0}, /* GPIO10 EBC_CS_N(5)      */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO11 EBC_BUS_ERR      */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO12 - SelectMAP INITn */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO13 - SelectMAP DONE */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO14 - SelectMAP PROGn */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO15 - SelectMAP RDWRn */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO16 GMCTxD(4)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO17 GMCTxD(5)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO18 GMCTxD(6)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO19 GMCTxD(7)      */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO20 RejectPkt0      */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO21 RejectPkt1      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO22 PB_KILLn */  \
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 SCPD0        */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1}, /* GPIO24 GMCTxD(2)      */  \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1}, /* GPIO25 GMCTxD(3)      */  \
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO26 - this is a weird one, the IIC function is really set in SDR0_PFC4*/  \
{GPIO0_BASE, GPIO_BI , GPIO_SEL , GPIO_OUT_1}, /* GPIO27 EXT_EBC_REQ  USB2D_RXERROR  - PPC_GPIO[1]*/  \
{GPIO0_BASE, GPIO_BI , GPIO_SEL , GPIO_OUT_1}, /* GPIO28    USB2D_TXVALID - PPC_GPIO[0] */  \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO29 EBC_EXT_HDLA  USB2D_PAD_SUSPNDM - PPC_GPIO[4] (Activity LED) */  \
{GPIO0_BASE, GPIO_BI , GPIO_SEL , GPIO_OUT_1}, /* GPIO30 EBC_EXT_ACK  USB2D_XCVRSELECT - PPC_GPIO[2]*/  \
{GPIO0_BASE, GPIO_BI , GPIO_SEL , GPIO_OUT_1}, /* GPIO31 EBC_EXR_BUSREQ  USB2D_TERMSELECT - PPC_GPIO[3]*/  \
}, \
{ \
/* GPIO Core 1 */ \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO32 USB2D_OPMODE0  EBC_DATA(2)  */  \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO33 USB2D_OPMODE1  EBC_DATA(3)  */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO34 UART0_8PIN_DCD_N UART1_DSR_CTS_N UART2_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO36 UART0_CTS_N  EBC_DATA(0)  UART3_SIN*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO37 UART0_RTS_N  EBC_DATA(1)  UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_1}, /* GPIO38 UART0_8PIN_DTR_N UART1_SOUT  */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_8PIN_RI_N UART1_SIN  */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 UIC_IRQ(0)  - GETH_IRQn */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 UIC_IRQ(1)  - FAULTn */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 UIC_IRQ(2)  - V6_IRQn */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 UIC_IRQ(3)  - THERM_ALERTn */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO44 UIC_IRQ(4)  DMA_ACK(1) - CPLD_IRQn */  \
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO45 UIC_IRQ(6)  DMA_EOT/TC(1) - RTC_IRQn */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO46 UIC_IRQ(7)  DMA_REQ(0) - MMC DMAREQ*/  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO47 UIC_IRQ(8)  DMA_ACK(0) - MMC DMAACK */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO48 UIC_IRQ(9)  DMA_EOT/TC(0) - MMC DMAEOT*/  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO49  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO55  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit  */  \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit  */  \
}                      \
}

#endif /* __CONFIG_H */


