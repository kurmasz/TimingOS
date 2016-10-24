#include "util.h"

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}


// Algorithm from www.codeproject.com/Articles/25172/Simple-Random-Number-Generation
static uint32_t m_w, m_z;
uint32_t random() { 
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  return (m_z << 16) + m_w;
}

void randomize(uint32_t seed) {
  if (seed == 0) {
    m_z = rdtsc();
  } else {
    m_z = seed;
  }
  m_w = m_z * m_z;
}
