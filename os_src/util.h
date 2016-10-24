#include "timingos.h"

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
   __asm__ volatile ("rdtsc" : "=A" (x));
  return x;
}


size_t strlen(const char* str);
uint32_t random();
void randomize(uint32_t seed);
