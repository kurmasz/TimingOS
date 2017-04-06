#include <stdio.h>
#include <stdbool.h>

#define MAGIC_STRING_LEN 5
char magic_string[]="QVWX ";


int main(int argc, char* argv[]) {


  // Search through the memory dump looking for the magic string.  To
  // keep the code simple, we are going to assume that the debug
  // buffer begins at a multiple of four.  This allows us to read data
  // in chunks of four bytes.  (Note, we don't use gets here becasue
  // we don't want the reading to stop at the end of "lines".)
  char buffer4[MAGIC_STRING_LEN];



  bool found_it;

  int i;
  do {
    found_it = true;
    for (i = 0; i < MAGIC_STRING_LEN; i++) {
      if (getchar() != magic_string[i]) {
	found_it = false;
      }
    } // end for
  } while (!found_it);

  // We have now found the beginning of the debug section. 
  // Just dump everything out
  char c;
  while ((c = getchar()) != EOF && c != '\0') {
    putc(c, stdout);
  }
}




  
