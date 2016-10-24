// I don't think we'll need this stuff
// #if !defined(__cplusplus)
// #include <stdbool.h> /* C doesn't have booleans by default. */
// #endif
#include <stddef.h>
#include <stdint.h>

/**********************************************************************
 * 
 * Standard checks that should be performed for all compilations 
 *
 ******************************************************************/

#if DEBUG
   #include <stdio.h>
   #include <stdlib.h>
#else

   /* Check if the compiler thinks we are targeting the wrong 
      operating system. */
   #if defined(__linux__)
      #error "You are not using a cross-compiler, you will most certainly run into trouble"
      #endif

   /* This tutorial will only work for the 32-bit ix86 targets. */
   #if !defined(__i386__)
   #error "This tutorial needs to be compiled with a ix86-elf compiler"
   #endif

#endif
