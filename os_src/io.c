#include "vga.h"
#include "util.h"
#include "memory_log.h"

void outb(int port, int data) {
  __asm__("out %%eax, %%dx" : : "a"(data), "d"(port));
}

/*
int inb(int port) {
  int answer;
  __asm__("inb %%dx, %%al\n" :"=a"(answer) : "d"(port) );
  return answer;
}
*/

static __inline unsigned char
inb (unsigned short int port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}


void ins(uint16_t cmd, char buffer[]) {

  __asm__("cld\n\t"
	  "rep insw" :
	  /* no output */
	  : "d"(cmd), "D"(buffer), "c"(256));
}


#if 0
void outsw(uint16_t cmd, char buffer[]) {
  __asm__("cld\n\t"
          "COMPACKLOOP:\n\t"
	  "mov $0x170, %%edx",
	  "lodsw\n\t"
          "out %%eax, %%edx"
	  "mov $0x3f6h, %%edx"
	  "in %%al, %%dx"
	  "loopnz compackloop" :
	  /* no output */
	  : "d"(cmd), "D"(buffer), "c"(6));
}
#endif


int PORT = 0x3f8;
int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void serial_write_char(char a) {
   while (is_transmit_empty() == 0);
 
   outb(PORT,a);
}


void serial_init()
{
  outb(PORT + 1, 0x00);// Disable all interrupts
  outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  outb(PORT + 0, 0x0C);    // Set divisor to 12 (lo byte) 9600 baud
  outb(PORT + 1, 0x00);    //                  (hi byte)
  outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
  outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_write(char output[]) {
  char* p = output;
  int count = 0;
  while (*p != '\0') {
    serial_write_char(*p);
    ++p;
    ++count;
  }
  return count;
}



int PRIMARY = 0x1F0;
int SECONDARY = 0x170;

int DATA_REG = 0;
int ERROR_REG = 1;
int DRIVE_HEAD_REG = 6;
int STATUS_REG = 7;
int ALTERNATE_REG = 511;

int ATTEMPTS = 1000;

int myio(char output[], int base_reg,
	  terminal_section_t *head, terminal_section_t *body) {

  terminal_write_string(body, "Let's go!\n");
  terminal_write_unsigned_hex(body, base_reg + STATUS_REG,  " Checking \n");
  unsigned val = inb(base_reg + STATUS_REG) & 0xff;
  //unsigned val = 6;
    terminal_write_unsigned_hex(body, val, 
				" <== value of status reg (waiting for !busy).\n");


#if 0

  int attempts = ATTEMPTS;
  unsigned val;
  do {
    val = inb(base_reg + STATUS_REG) & 0xff;
    terminal_write_unsigned_hex(body, val, 
				" <== value of status reg (waiting for !busy).\n");

    // Check for empty bus
    if (val == 0xff) {
      terminal_write_unsigned_hex(head, val, " No drives\n");
      return 0;
    }

    // Check error status.
    if (val & 0x1) {
      int v2 = inb(base_reg+ERROR_REG) & 0xff;
      terminal_write_unsigned_hex(body, v2, " <== value of error reg.\n");

      v2 = inb(base_reg+ALTERNATE_REG) & 0xff;
      terminal_write_unsigned_hex(body, v2, " <== value of alternate reg.\n");
    }

    // Eventually give up.
    if (--attempts == 0) {
      terminal_write_unsigned_hex(head, base_reg, " I give up (waiting for !busy).\n");
      return 0;
     }
   }  while(val & 0x80);
   terminal_write_string(body, "Not busy\n");


   attempts = ATTEMPTS;
   do {
     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for DRDY)\n ");
     if (val & 0x1) {
       int v2 = inb(base_reg + ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== error on wait for ready\n");

       int v3 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v3, " <== value of alternate reg.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting drdy).\n");
       return 0;
     }
   } 
   while(!(val & 0x40)); // we are looking for a 1 in bit position 6
   terminal_write_string(head, "ATA ready\n");

   // ******************
   outb(base_reg + DRIVE_HEAD_REG, 0x0); // 0, select master 0x10 selects slave
   outb(base_reg + STATUS_REG, 0xec);  // 0xEC is identify drive 

   attempts = ATTEMPTS;
   do {

     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for DRQ)\n ");
     if (val & 0x1) {
       int v2 = inb(base_reg + ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== error on wait for DRQ\n");

       int v3 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v3, " <== value of alternate reg on wait for DRQ.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting DRQ).\n");
     }
   }while(!(val & 0x8) && !(val & 0x1));


   attempts = 0;
   if (val & 0x8) {
     terminal_write_string(head, "Data Ready!!\n");
     ins(base_reg + DATA_REG, output);
     while (*output == '\0') {
       terminal_write_unsigned(body, ++attempts,  "Is zero!\n");
     }
     return 1;
   } else if (val & 0x1) {
     terminal_write_string(body, "Wait on DRQ failed\n");
     terminal_write_unsigned_hex(body, val, "<== status word\n");
     int nv = inb(base_reg + ERROR_REG) &0xff;
     terminal_write_unsigned_hex(body, nv, "<== Error word\n--\n");
     return 0;
   }
#endif
   return 0;
}


 int myio_atapi(char  output [], int base_reg,
	   terminal_section_t *head, terminal_section_t *body) {

   output[0] = '!';
   terminal_write_string(body, "Let's try atapi!\n");
   int attempts = ATTEMPTS;
   unsigned char val;
   do {
     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for !busy).\n");

     // Check for empty bus
     if (val == 0xff) {
       terminal_write_unsigned_hex(head, val, " No drives\n");
       return 0;
     }

     // Check error status.
     if (val & 0x1) {
       int v2 = inb(base_reg+ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== value of error reg.\n");

       v2 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== value of alternate reg.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting for !busy).\n");
       return 0;
     }
   }  while(val & 0x80);
   terminal_write_string(body, "ATAPI Not busy\n");


   attempts = ATTEMPTS;
   do {
     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for DRDY)\n ");
     if (val & 0x1) {
       int v2 = inb(base_reg + ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== error on wait for ready\n");

       int v3 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v3, " <== value of alternate reg.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting drdy).\n");
       return 0;
     }
   } 
   while(!(val & 0x40)); // we are looking for a 1 in bit position 6
   terminal_write_string(head, "ATAPI ready\n");

   // ******************
   outb(base_reg + DRIVE_HEAD_REG, 0x10); // 0, select master 0x10 selects slave
   outb(ALTERNATE_REG, 0x0a); // Set nIEN (whatever that means)
   outb(base_reg + STATUS_REG, 0xA0);  // Send the PACKET command

   unsigned dummy = 3;
   for (int i = 0; i < 10000; i++) {
     dummy = dummy*(dummy-1);
   }
//   debug_write_unsigned(dummy, " Dummy value");

   // Wait for busy again.
   do {
     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for !busy).\n");

     // Check for empty bus
     if (val == 0xff) {
       terminal_write_unsigned_hex(head, val, " No drives\n");
       return 0;
     }

     // Check error status.
     if (val & 0x1) {
       int v2 = inb(base_reg+ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== value of error reg.\n");

       v2 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== value of alternate reg.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting for !busy).\n");
       return 0;
     }
   }  while(val & 0x80);
   terminal_write_string(body, "ATAPI Not busy (take 2)\n");


   // Wait for DRQ to clear.
   attempts = ATTEMPTS;
   do {

     val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== value of status reg (waiting for DRQ)\n ");
     if (val & 0x1) {
       int v2 = inb(base_reg + ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== error on wait for DRQ\n");

       int v3 = inb(base_reg+ALTERNATE_REG) & 0xff;
       terminal_write_unsigned_hex(body, v3, " <== value of alternate reg on wait for DRQ.\n");
     }

     // Eventually give up.
     if (--attempts == 0) {
       terminal_write_unsigned_hex(head, base_reg, " I give up (waiting DRQ).\n");
     }
   }while(!(val & 0x8) && !(val & 0x1));
   terminal_write_string(body, "DRQ is ready.  Time to send packet.\n");


   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, "  1\n");
   outb(base_reg + STATUS_REG, 0);
     terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 2\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 3\n");
   outb(base_reg + STATUS_REG, 2);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 4\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 5\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 6\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 7\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 8\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 9\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 10\n");
   outb(base_reg + STATUS_REG, 0);
   terminal_write_unsigned_hex(body, inb(base_reg + ALTERNATE_REG) & 0xff, " 11\n");

   terminal_write_string(head, "Atapi done\n");

   val = inb(base_reg + STATUS_REG) & 0xff;
     terminal_write_unsigned_hex(body, val, 
				 " <== Final status\n ");
     // Check error status.
     if (val & 0x1) {
       int v2 = inb(base_reg+ERROR_REG) & 0xff;
       terminal_write_unsigned_hex(body, v2, " <== value of error reg.\n");
     }

   return 0;
 }




 void io_kernel_main() {
   randomize(0);
   /* Initialize terminal interface */
   terminal_initialize();

   terminal_section_t head, body;
   int lines_in_head = 5;
   terminal_section_initialize(&head, 0, lines_in_head-1, 0, VGA_WIDTH-1);
   terminal_section_initialize(&body, lines_in_head, 
			       VGA_HEIGHT-1, 0, VGA_WIDTH-1);
   terminal_write_string(&head, "IO Testing\n");
   terminal_write_string(&body, "===Debug I/O===\n");

   /*
   unsigned long long debug_loc = debug_init();

   terminal_write_unsigned_hex(&head, debug_loc, " Address of debug\n");

   debug_write_string("Well, Hello there, everybody.\n");
   debug_write_string("It's a good day for debugging.\n");
   */

 #if 0
   serial_init();

   serial_write("AAAAAAA");
   serial_write("Hello, World!");
   serial_write("abcdefghijklmnop");
   serial_write("\n\n\n");
   unsigned count = 0;
   while (count < 1000) {
     int x = serial_write("AAAAAAA abcdefghijklmnop");
     terminal_write_unsigned(&body, x, "Count");
     terminal_write_unsigned(&body, ++count, "\n");
   }
 #endif


   char bar[1024];
   myio_atapi(bar, PRIMARY, &head, &body);

  char foo[1024] = "abcdefghigjlmnopqrstuvwxyz";
  unsigned* buffer = (unsigned*)foo;
  if (myio(foo, SECONDARY, &head, &body) != 0) {
  terminal_write_string(&body, "(");
  for (int i = 0; i < 128; i++) {
    terminal_write_unsigned(&body, i, " ");
    if (foo[i] >= 0x20 && foo[i] <= 0x7e) {
      terminal_putchar(&body, foo[i]);
    } else if (foo[i] == 0) {
      terminal_write_string(&body, "0");
    } else {
      terminal_write_unsigned_hex(&body, (unsigned)foo[i], "");
    }
    terminal_write_string(&body, ") (");
  } // end for
  } // end if there is no error


  /*
  for (int i = 0; i < 128; i++) {
    terminal_write_unsigned(&body, i, " ");
    terminal_write_unsigned_hex(&body, buffer[i], ") (");
  }
  */
  //  terminal_write_string(&body, "Dumping foo:\n");
  //terminal_write_string(&body, foo);
  //terminal_write_string(&body, "Foo dumped:\n");
}
