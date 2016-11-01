// This is the code that is run when running the "os" from the command line
#if DEBUG
#include "vga.h"

void KERNEL_MAIN();
int main() {
  KERNEL_MAIN();

  /* When run as a normal program, it is not possible to write
   directly to the screen Calling terminal_dump dumps the print buffer
   to the command-line terminal using printf.
  */
  terminal_dump();
  return 0;
}
#endif
