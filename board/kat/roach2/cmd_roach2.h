#ifndef _CMD_ROACH2_H_
#define _CMD_ROACH2_H_

#ifdef CONFIG_CMD_ROACH2_SMAP

#define GPIO_SMAP_INITN 12
#define GPIO_SMAP_DONE  13
#define GPIO_SMAP_PROGN 14
#define GPIO_SMAP_RDWRN 15

/* 32 bit word offset of the sync symbol in the bitsream */
#define SMAP_SYNC_OFFSET  0xc
#define SMAP_SYNC_VALUE   0xaa995566

/* Delay fors smap checks */
#define SMAP_INITN_WAIT 100000
#define SMAP_DONE_WAIT  100000

/* Default SX475T image size in bytes */
#define SMAP_IMAGE_SIZE 19586188

#endif /* CONFIG_CMD_ROACH2_SMAP */

#endif /* __CMD_ROACH2_H__ */
