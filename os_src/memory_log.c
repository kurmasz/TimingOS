#include "timingos.h"

#define DEBUG_BUFFER_SIZE 1048576
static char debug_buffer[DEBUG_BUFFER_SIZE];
static int debug_buffer_index = 0;

void debug_write_string(const char *string) {
  const char* p = string;
  while (*p != '\0') {
    debug_buffer[debug_buffer_index++] = *p;
    ++p;
  }
}

#define WRITEINT_BUFFER 100
void debug_write_unsigned(unsigned long long value, const char* data) {
static char buffer[WRITEINT_BUFFER+1];
int place = WRITEINT_BUFFER;
buffer[place] = '\0';
  if (value == 0) {
debug_write_string("0");
  } else {
    while (value > 0) {
      buffer[--place] = (value % 10) + '0';
      value /= 10;
    }
  } // end else

debug_write_string(buffer+place);

  if (data != NULL && data[0] != 0) {
debug_write_string(data);
  }
}


void debug_write_unsigned_hex(unsigned long long value, const char* data) {
  static char buffer[WRITEINT_BUFFER+1];
  int place = WRITEINT_BUFFER;
buffer[place] = '\0';

  if (value == 0) {
    debug_write_string("0x0");
  } else {
    debug_write_string("0x");
    while (value > 0) {
      if ((value % 16) < 10) {
	buffer[--place] = (value % 16) + '0';
      } else {
	buffer[--place] = (value % 16) + 'a' - 10;
      }
      value /= 16;
    }
  } // end else

debug_write_string(buffer + place);

  if (data != NULL && data[0] != 0) {
debug_write_string(data);
  }
}





unsigned long long debug_init() {

  debug_buffer_index = DEBUG_BUFFER_SIZE - 15;
  debug_write_string("XWVQ Debug End");

  // These are written one-by-one to make the string 'QVWX' unique 
  // in the memory dump.  If we hard-coded it as a string, it would 
  // likely appear in the instruction memory as well.
  debug_buffer[0] = 'Q';
  debug_buffer[1] = 'V';
  debug_buffer[2] = 'W';
  debug_buffer[3] = 'X';
  debug_buffer[4] = ' ';

  debug_buffer_index = 5;
  debug_write_string("Debug Start\n");

  return (unsigned long long)debug_buffer;
}


