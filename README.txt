TimingOS is a *very* simple Operating System designed to help students
accurately time code (without interference from interrupts, context
switches, etc.)

Basically, the students write C code they want to time, then compile
it right into the kernel of a "minOS".  This "mini OS" is built into
an *.iso image that, when booted from, runs the student's code and
nothing else.

TimingOS provides almost nothing: No printf, no malloc, no glibc, no
disk I/O.  Students have acces to only: 
   (1) a very basic random number generator,
   (2) very primitive text I/O (on a 25x80 VGA text screen), and	
   (3) a C interfact to the Read Timestamp Counter (rdtsc) instruction.


Begin by looking a the 'hello_world.c' sample.  Then, 
   (1) rename `kernel_main` in `hello_world.c`.
   (2) Create a new .c file in the usr_src directory that contains a function
       named `kernel_main`.
   (3) Run 'make'.  Your mini-OS will be in `timingos.iso`.
   (4) Copy `timingos.iso` to an empty USB thumb drive and boot from it.

