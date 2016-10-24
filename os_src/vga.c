#include "timingos.h"
#include "util.h"
#include "vga.h"



uint8_t make_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color) {
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;
uint16_t* terminal_buffer;

void terminal_initialize() {

  uint8_t terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
#if DEBUG
  terminal_buffer = (uint16_t*) malloc(VGA_HEIGHT*VGA_WIDTH*sizeof(uint16_t));
#else
  terminal_buffer = (uint16_t*) 0xB8000;
#endif 

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = make_vgaentry(' ', terminal_color);
    }
  }
}

void terminal_section_initialize(terminal_section_t* section, 
				 size_t start_row, size_t bottom_row, 
				 size_t start_column, size_t right_column) {
  section->top_row = start_row;
  section->bottom_row = bottom_row;
  section->left_column = start_column;
  section->right_column = right_column;

  section->current_row = start_row;
  section->current_column = start_column;

  section->current_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
}


void terminal_setcolor(terminal_section_t* section, uint8_t color) {
	section->current_color = color;
}

// Place globally
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}


void terminal_putchar(terminal_section_t* section, char c) {
  if (c == '\n') {
    for (;section->current_column <= section->right_column; ++(section->current_column)) {
        terminal_putentryat(' ', section->current_color, 
			    section->current_column, section->current_row);
    }
    section->current_column = section->left_column;
    ++(section->current_row);
    if (section->current_row > section->bottom_row) {
      section->current_row = section->top_row;
    }
    return;
  }
  terminal_putentryat(c, section->current_color, 
		      section->current_column, section->current_row );
  ++(section->current_column);
  if (section->current_column > section->right_column) {
    section->current_column = section->left_column;
    ++(section->current_row);
    if (section->current_row > section->bottom_row ) {
      section->current_row = section->top_row;
    }
  }
}

void terminal_write_string(terminal_section_t* section, const char* data) {
	size_t datalen = strlen(data);
	for (size_t i = 0; i < datalen; i++)
	  terminal_putchar(section, data[i]);
}

#define WRITEINT_BUFFER 100
void terminal_write_unsigned(terminal_section_t* section, unsigned long long value, const char* data) {
  static char buffer[WRITEINT_BUFFER];
  int place = WRITEINT_BUFFER;

  if (value == 0) {
    terminal_putchar(section, '0');
  } else {
    while (value > 0) {
      buffer[--place] = (value % 10) + '0';
      value /= 10;
    }

    for(; place < WRITEINT_BUFFER; place++) {
      terminal_putchar(section, buffer[place]);
    }
  } // end else


  if (data != NULL && data[0] != 0) {
    terminal_write_string(section, data);
  }
}


void terminal_write_unsigned_hex(terminal_section_t* section, unsigned long long value, const char* data) {
  static char buffer[WRITEINT_BUFFER];
  int place = WRITEINT_BUFFER;

  if (value == 0) {
    terminal_write_string(section, "0x0");
  } else {
    terminal_write_string(section, "0x");
    while (value > 0) {
      if ((value % 16) < 10) {
	buffer[--place] = (value % 16) + '0';
      } else {
	buffer[--place] = (value % 16) + 'a' - 10;
      }
      value /= 16;
    }

    for(; place < WRITEINT_BUFFER; place++) {
      terminal_putchar(section, buffer[place]);
    }
  } // end else


  if (data != NULL && data[0] != 0) {
    terminal_write_string(section, data);
  }
}



void terminal_write_signed(terminal_section_t* section, signed long long value, const char* data) {
  if (value < 0) {
    terminal_putchar(section, '-');
    value = -value;
  }
  terminal_write_unsigned(section, (unsigned long long)value, data);
}


#if DEBUG
void terminal_dump() {
  char buffer[VGA_WIDTH*VGA_HEIGHT];
  for (int i = 0; i < VGA_WIDTH*VGA_HEIGHT; i++) {
    buffer[i] = (char) terminal_buffer[i];
  }
  for (int row = 0; row < VGA_HEIGHT; row++) {
    buffer[row*VGA_WIDTH + VGA_WIDTH -1] = '\0';
    puts(buffer + row*VGA_WIDTH);
  }
}
#endif
