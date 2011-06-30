#ifndef _ROACH2_SMAP_H_
#define _ROACH2_SMAP_H_

struct bit_mapping {
  int (*test) (int which, int subtest, u32 flags);
  const char* name;
  uint devices;
  uint subtests;
  const char** subtest_name;
};

extern char bit_strerr[256];

#endif /* __CMD_ROACH2_H__ */
