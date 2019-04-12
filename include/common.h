#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>

// Just prints errors
void error(const char *msg);

// returns unix sysetm time as 64 bit nanoseconds
uint64_t get_unix_time();

#endif