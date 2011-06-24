#ifndef ROACH2_CPLD_H_
#define ROACH2_CPLD_H_

/* Registers */

#define CPLD_LEDS       0x5

#define CPLD_SM_STATUS  0x8
#define CPLD_SM_OREGS   0x9
#define CPLD_SM_DATA    0xa
#define CPLD_SM_CTRL    0xb

#define CPLD_REV_ID     0x18
#define CPLD_REV_MAJOR  0x1a
#define CPLD_REV_MINOR  0x1b
#define CPLD_REV_RCS    0x1c

/*MMC*/
#define  MMC_DATA_O        0x10 
#define  MMC_DATA_I        0x11
#define  MMC_CMD_O         0x12 
#define  MMC_CMD_I         0x13 
#define  MMC_OENS          0x14
#define  MMC_STATUS        0x15
#define  REG_ADV_TYPE      0x16 
#define  REG_ADV_MAN       0x17

/* Flags */

#define CPLD_SM_DONE    0x2
#define CPLD_SM_INIT_N  0x1

/* Misc */

#define CPLD_ID         0xd00d

#endif

