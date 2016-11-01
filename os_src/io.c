#include "vga.h"
#include "util.h"

void outb(int port, int data) {
  __asm__("out %%eax, %%dx" : : "a"(data), "d"(port));
}

int inb(int port) {
  int answer;
  __asm__("in %%dx, %%eax\n" :"=a"(answer) : "d"(port) );
  return answer;
}

void ins(uint16_t cmd, char* buffer) {

  __asm__("cld\n\t"
	  "rep insw" :
	  /* no output */
	  : "d"(cmd), "D"(buffer), "c"(256));
}


void myio(char* output, terminal_section_t *body) {

  terminal_write_string(body, "Let's go!\n");
  unsigned val;
  do {
    val = inb(0x1f7);
    terminal_write_unsigned_hex(body, val, " <== wait for not busy\n");
    if (val & 0x1) {
      int v2 = inb(0x1f1);
      terminal_write_unsigned_hex(body, v2, " <== error on wait\n");
    }
  }  while(val & 0x80);
  terminal_write_string(body, "Not busy\n\n");


  do {
    val = inb(0x1f7);
    terminal_write_unsigned_hex(body, val, " <== wait for dev ready\n ");
    if (val & 0x1) {
      int v2 = inb(0x1f1);
      terminal_write_unsigned_hex(body, v2, " <== error on wait for ready\n");
    }
  } 
  while(!(val & 0x40));
  terminal_write_string(body, "ATA ready\n\n");

  outb(0x1f6, 0);
  outb(0x1f7, 0xec);

  int count = 0;
  unsigned pv = 1000000;
  do {
    val = inb(0x1f7);
    if (val != pv) {
      terminal_write_unsigned_hex(body, val, "<== status word\n");
    }
    pv = val;
    ++count;
  }while(!(val & 0x8) && !(val & 0x1));

  if (val & 0x8) {
    terminal_write_string(body, "\nSuccess!!\n");
    ins(0x1f7, output);
    terminal_write_string(body, "\nDone!!\n");
  } else {
    terminal_write_string(body, "\nFail!\n");
    terminal_write_unsigned_hex(body, val, "<== status word\n");
    int nv = inb(0x1f1);
    terminal_write_unsigned_hex(body, nv, "<== Error word\n");
  }
}

void kernel_main() {
  randomize(0);
  /* Initialize terminal interface */
  terminal_initialize();

  terminal_section_t head, body;
  terminal_section_initialize(&head, 0, 3, 0, VGA_WIDTH-1);
  terminal_section_initialize(&body, 4, VGA_HEIGHT-1, 0, VGA_WIDTH-1);
  terminal_write_string(&head, "Branch Predictor Testing (9)\n");
  terminal_write_string(&body, "Data:\n");


  /*
  unsigned a = 0;
  asm("smsw %0" : "=r" (a));
  terminal_write_unsigned(&head, a, "\n");
  terminal_write_string(&head, "Done  (2)!");
  */
  char foo[1024];
  myio(foo, &body);
  terminal_write_string(&body, foo);

}
