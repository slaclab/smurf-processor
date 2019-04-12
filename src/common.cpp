#include "common.h"

// Just prints errors
void error(const char *msg)
{
  perror(msg);
}

// returns unix sysetm time as 64 bit nanoseconds
uint64_t get_unix_time()
{
  timespec tmp_t;  // structure seconds, nanoseconds
  uint64_t tmp;
  //clock_gettime(CLOCK_REALTIME, &tmp_t);  // get time s, ns,  might be expensive
  clock_gettime(CLOCK_REALTIME, &tmp_t);
  tmp = 1000000000l * (uint64_t) tmp_t.tv_sec + (uint64_t) tmp_t.tv_nsec;  //  multiply to 64 uint
  return(tmp);
}