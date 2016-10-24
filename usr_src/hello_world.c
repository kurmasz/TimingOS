#include "timingos.h"
#include "util.h"
#include "vga.h"

/****************************************************************
 * Sample "user" code for TimingOS.
 * 
 * Notice that 
 *   (1) You are responsible for setting up the terminal.  
 *       You divide the screen into one or more text areas.
 *       When you reach the bottom of the area, the text
 *       wraps around back to the top.  Having multiple 
 *       areas lets you send "debug" info to one part of the
 *       screen and "final answers" to another.
 *
 *   (2) You have access to only those functions provided by
 *       TimingOS.  (Look at os_src / *.h).  You don't have 
 *       any standard C libraries (no printf, no malloc, etc.)
 *
 * To write your own code, create a new file in this directory,
 * Add a function named "kernel_main", and rename the kernel_main
 * below.  (If you don't rename kernel_main below, you will get
 * "duplicate symbol" errors.)
 ****************************************************************/


void kernel_main() {
  randomize(0);
  
  /* Initialize terminal interface.  This function must always be
     called.  It sets up the text buffer. 
  */
  terminal_initialize();  

  /* Now, set up the individual text areas.  The code here sets up 
     two text areas:  The first four lines of the screen are labeled
     "head", the remainder of the screen is labeled "body"
  */
  terminal_section_t head, body;
  terminal_section_initialize(&head, 0, 3, 0, VGA_WIDTH-1);
  terminal_section_initialize(&body, 4, VGA_HEIGHT-1, 0, VGA_WIDTH-1);
  terminal_write_string(&head, "Hello, world\n");
  terminal_write_string(&body, "Data Section:\n");

  for (int i = 0; i < 137; i++) {
    terminal_write_string(&body, "This is body line ");
    terminal_write_unsigned(&body, i, "\n");
  }
  terminal_write_string(&body, "Done writing in the body\n");
  
  terminal_write_string(&head, "This is another line in the head\n");
  terminal_write_string(&head, "Good bye.\n");
}

// This is the code that is run when running the "os" from the command line
#if DEBUG
int main() {
  kernel_main();

  /* When run as a normal program, it is not possible to write
   directly to the screen Calling terminal_dump dumps the print buffer
   to the command-line terminal using printf.
  */
  terminal_dump();
  return 0;
}
#endif
