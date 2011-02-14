#ifndef _CMD_ROACH2_H_
#define _CMD_ROACH2_H_

#ifdef CONFIG_CMD_ROACH2_SMAP

#define GPIO_SMAP_INITN 12
#define GPIO_SMAP_DONE  13
#define GPIO_SMAP_PROGN 14
#define GPIO_SMAP_RDWRN 15
#define GPIO_SMAP_GPIO0 28
#define GPIO_SMAP_GPIO1 27
#define GPIO_SMAP_GPIO2 30
#define GPIO_SMAP_GPIO3 31
#define GPIO_SMAP_LED   29

struct gpio_mapping{
  const char* name;
  int index;
};

#define GPIO_COUNT 9
struct gpio_mapping gpio_map [GPIO_COUNT] = {
  {"init_n", GPIO_SMAP_INITN},
  {"done",   GPIO_SMAP_DONE },
  {"prog_n", GPIO_SMAP_PROGN},
  {"rdwr_n", GPIO_SMAP_RDWRN},
  {"gpio0",  GPIO_SMAP_GPIO0},
  {"gpio1",  GPIO_SMAP_GPIO1},
  {"gpio2",  GPIO_SMAP_GPIO2},
  {"gpio3",  GPIO_SMAP_GPIO3},
  {"actled", GPIO_SMAP_LED  } };

/* 32 bit word offset of the sync symbol in the bitsream */
#define SMAP_SYNC_OFFSET_BIN  48
#define SMAP_SYNC_OFFSET_BIT  146
#define SMAP_SYNC_VALUE   0xaa995566

/* Delay fors smap checks */
#define SMAP_INITN_WAIT 100000
#define SMAP_DONE_WAIT  100000

/* Default SX475T image size in bytes */
#define SMAP_IMAGE_SIZE 19586188

#endif /* CONFIG_CMD_ROACH2_SMAP */

#endif /* __CMD_ROACH2_H__ */
