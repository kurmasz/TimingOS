#include "timingos.h"

const size_t VGA_WIDTH;
const size_t VGA_HEIGHT;


/* Hardware text mode color constants. */
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

typedef struct {

  // Indexed to full screen
  size_t top_row, bottom_row;
  size_t left_column, right_column;

  // Zero indexed
  size_t current_row;
  size_t current_column;


  uint8_t current_color;
} terminal_section_t;




void terminal_initialize();

uint8_t make_color(enum vga_color fg, enum vga_color bg);
void terminal_setcolor(terminal_section_t* section, uint8_t color);
void terminal_putchar(terminal_section_t* section, char c);
void terminal_write_string(terminal_section_t* section, const char* data);
void terminal_write_unsigned(terminal_section_t* section, unsigned long long value, const char* data);
void terminal_write_unsigned_hex(terminal_section_t* section, unsigned long long value, const char* data);
void terminal_write_signed(terminal_section_t* section, signed long long value, const char* data);

void terminal_section_initialize(terminal_section_t* section, 
				 size_t start_row, size_t height, 
				 size_t start_column, size_t width);

#if DEBUG
void terminal_dump();
#endif
