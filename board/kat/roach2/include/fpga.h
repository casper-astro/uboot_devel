#ifndef _ROACH2_FPGA_H_
#define _ROACH2_FPGA_H_


#define BSP_QDR3_OFFSET     0x03800000
#define BSP_QDR2_OFFSET     0x03000000
#define BSP_QDR1_OFFSET     0x02800000
#define BSP_QDR0_OFFSET     0x02000000
#define BSP_ZDOK1_OFFSET    0x00090000
#define BSP_ZDOK0_OFFSET    0x00080000
#define BSP_QDR3CONF_OFFSET 0x00060000
#define BSP_QDR2CONF_OFFSET 0x00050000
#define BSP_QDR1CONF_OFFSET 0x00040000
#define BSP_QDR0CONF_OFFSET 0x00030000

/* Basic comms definitions */
#define BSP_REG_BOARDID  0x0
#define BSP_REG_REVMAJ   0x4
#define BSP_REG_REVMIN   0x8
#define BSP_REG_REVRCS   0xc
#define BSP_REG_SCRATCH(x) (0x10 + (((x) * 4) & 0xf))

#define BSP_BOARDID 0xdeadcafe

/* QDR mem software interface */

#define BSP_QDRM_REG_STAT 0x0
#define BSP_QDRM_REG_CTRL 0x4
#define BSP_QDRM_REG_A    0x8
#define BSP_QDRM_REG_D    0x20
#define BSP_QDRM_REG_Q    0x40
#define BSP_QDRM_DLEN     0x5

#define BSP_QDRM_PHYRDY   0x1
#define BSP_QDRM_CALFAIL  0x100
#define BSP_QDRM_RDEN     0x001
#define BSP_QDRM_WREN     0x100

/* QDR memory depth (number of 144B entries) */
//#define BSP_QDRM_DEPTH    (512*1024)

#define BSP_QDRM_DEPTH    (1024)

/* GPIO controller */
#define BSP_GPIO_INPUT  0x0
#define BSP_GPIO_OUTPUT 0x1
#define BSP_GPIO_VAL_0  0x0
#define BSP_GPIO_VAL_1  0x1

/* QDR Config w/ software cal */
#define BSP_QDRCONF_REG_CTRL     0x0
#define BSP_QDRCONF_REG_BITINDEX 0x4
#define BSP_QDRCONF_REG_BITCTRL  0x8
#define BSP_QDRCONF_REG_STATUS   0xc

#define BSP_QDRCONF_DLL_RESET    0x00000000
#define BSP_QDRCONF_DLL_RUN      0x00000100       

#define BSP_QDRCONF_CAL_EN (0x00000001)
#define BSP_QDRCONF_CAL_DIS (0x00000000)

#define BSP_QDRCONF_CAL_RDY    (0x01000000)
#define BSP_QDRCONF_DAT_RDY    (0x00010000)
#define BSP_QDRCONF_DAT_STABLE (0x00000100)

#define BSP_QDRCONF_DLY_RST (0x00000004)
#define BSP_QDRCONF_DLY_EN  (0x00000001)
#define BSP_QDRCONF_DLY_INC (0x00000002)
#define BSP_QDRCONF_DLY_DEC (0x00000000)

#define BSP_QDRCONF_ALIGN_EN      (0x00000100)
#define BSP_QDRCONF_ALIGN_BYPASS  (0x00000000)

/* DDR3 definitions */

#define BSP_DDR3_OFFSET     0x4000000

#define BSP_DDR3_REG_STATUS (BSP_DDR3_OFFSET + 0x0)
#define BSP_DDR3_REG_CTRL   (BSP_DDR3_OFFSET + 0x4)
#define BSP_DDR3_REG_ADDR   (BSP_DDR3_OFFSET + 0x8)

#define BSP_DDR3_PHY_RDY    0x000000001
#define BSP_DDR3_RD         0x000000001
#define BSP_DDR3_WR         0x000000100

#define BSP_DDR3_REG_WR(x)  (BSP_DDR3_OFFSET + (0x080 + ((x)/5)*32 + 4*((x)%5)))
#define BSP_DDR3_REG_RD(x)  (BSP_DDR3_OFFSET + (0x100 + ((x)/5)*32 + 4*((x)%5)))

#define BSP_DDR3_DLEN     (5*4)

#define BSP_DDR3_IS_HALF(x)  ((x%5)==0)

/* GBE definitions */

#define BSP_GBE_OFFSET     0x600000

#define BSP_GBE_REG_OFFSET 0x0
#define BSP_GBE_TX_OFFSET  0x1000
#define BSP_GBE_RX_OFFSET  0x2000
#define BSP_GBE_ARP_OFFSET 0x3000

#define BSP_GBE_REG_TXSIZE 0x18
#define BSP_GBE_REG_RXSIZE 0x1a
#define BSP_GBE_REG_STIDLE 0x24
#define BSP_GBE_REG_STLINK 0x26

/* TGE definitions */

#define BSP_TGE_OFFSET(x)  (0x200000 + 0x80000*(x))

#define BSP_TGE_REG_OFFSET 0x0
#define BSP_TGE_TX_OFFSET  0x1000
#define BSP_TGE_RX_OFFSET  0x2000
#define BSP_TGE_ARP_OFFSET 0x3000

#define BSP_TGE_REG_TXSIZE 0x18
#define BSP_TGE_REG_RXSIZE 0x1a
#define BSP_TGE_REG_STIDLE 0x24
#define BSP_TGE_REG_STLINK 0x26

#endif
