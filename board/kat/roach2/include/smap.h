#ifndef _ROACH2_SMAP_H_
#define _ROACH2_SMAP_H_

#define GPIO_SMAP_INITN 12
#define GPIO_SMAP_DONE  13
#define GPIO_SMAP_PROGN 14
#define GPIO_SMAP_RDWRN 15
#define GPIO_SMAP_GPIO0 28
#define GPIO_SMAP_GPIO1 27
#define GPIO_SMAP_GPIO2 30
#define GPIO_SMAP_GPIO3 31
#define GPIO_SMAP_LED   29
#define GPIO_SMAP_KILLN 22

/* 32 bit word offset of the sync symbol in the bitsream */
#define SMAP_SYNC_OFFSET_BIN  48
#define SMAP_SYNC_OFFSET_BIT  146
#define SMAP_SYNC_VALUE   0xaa995566

/* Delay fors smap checks */
#define SMAP_INITN_WAIT 100000
#define SMAP_DONE_WAIT  100000

/* Default SX475T image size in bytes */
#define SMAP_IMAGE_SIZE 19586188

#endif /* __CMD_ROACH2_H__ */
