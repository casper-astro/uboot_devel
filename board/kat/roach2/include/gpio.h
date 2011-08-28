#ifndef _ROACH2_GPIO_H_
#define _ROACH2_GPIO_H_

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
#define GPIO_SMAP_FAULTN 41 

struct gpio_mapping{
  const char* name;
  int index;
};

#define GPIO_COUNT 11

#define R2_GPIO_MAPPING { \
{"faultn", GPIO_SMAP_FAULTN}, \
{"init_n", GPIO_SMAP_INITN}, \
{"done",   GPIO_SMAP_DONE }, \
{"prog_n", GPIO_SMAP_PROGN}, \
{"rdwr_n", GPIO_SMAP_RDWRN}, \
{"gpio0",  GPIO_SMAP_GPIO0}, \
{"gpio1",  GPIO_SMAP_GPIO1}, \
{"gpio2",  GPIO_SMAP_GPIO2}, \
{"gpio3",  GPIO_SMAP_GPIO3}, \
{"actled", GPIO_SMAP_LED  }, \
{"killn",  GPIO_SMAP_KILLN}, \
}

#endif
