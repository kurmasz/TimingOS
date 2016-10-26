###############################################################################
#
# Makefile for TimingOS
#
# (C) 2016 Zachary Kurmas
#
# This Makefile requires that m4, grub (specifically, grub2-mkrescue), 
# i686-elf-gcc, and other necessary toolchain compnents, be in the PATH.
#
# User code goes in usr_src.  This directory must contain exactly one .c file
# with a kernel_main function. 
#
###############################################################################

# makefile.local should contain the definition for KERNEL_MAIN
# Begin by loading makefile.local.sample. If makefile.local exists, 
# it will overwrite the values set in makefile.local.sample.  If it doesn't
# exist, nothing will happen.
include makefile.local.sample
-include makefile.local

OS_SRC  = os_src
USR_SRC = usr_src
OS_OBJ = obj/os
USR_OBJ = obj/usr
elfCC = i686-elf-gcc
CFLAGS = -std=c99 -ffreestanding -Wall -Wextra

os_sources = $(wildcard $(OS_SRC)/*.c)      # all *.c files in the os_src dir
usr_sources = $(wildcard $(USR_SRC)/*.c)    # all *.c files in the usr_src dir

# Convert each .c file name into the corresponding .o file name
# (change .c to .o, and change the directory name)
os_objs = $(subst $(OS_SRC), $(OS_OBJ), $(os_sources:.c=.o))
usr_objs = $(subst $(USR_SRC), $(USR_OBJ), $(usr_sources:.c=.o))

all_objs = $(os_objs) $(OS_OBJ)/boot.o $(usr_objs)


# timingos.iso is a bootable iso image intended to be run on "real" hardware
# timingos.debug is an executable program designed to be run for debugging
all: timingos.iso timingos.debug

#
# Make the obj directories, if they don't already exist.
#
$(OS_OBJ):
	mkdir -p $@

$(USR_OBJ):
	mkdir -p $@

#
# create makefile.local if it doesn't already exist
#
makefile.local:
	cp makefile.local.sample $@


#############################################################################
#
# Compile "core" OS
#
#############################################################################

# Headers shouldn't change much.  If they do, then just 
# re-build all the .o files
$(OS_OBJ)/%.o: $(OS_SRC)/%.c $(OS_SRC)/*.h | $(OS_OBJ)
	$(elfCC) $(CFLAGS) -c $< -o $@ -O2
        # generate the assembly file, in case we want to look at it later.
	$(elfCC) $(CFLAGS) -S $< -o $(@:.o=.s) -O2 


# By naming the file boot.S (instead of boot.s), gcc will run the 
# pre-processor and substitute KERNEL_MAIN with the user-chosen
# kernel entry point.
$(OS_OBJ)/boot.o: $(OS_SRC)/boot.S | $(OS_OBJ) makefile.local
	mkdir -p $(@D)
	$(elfCC) $(CFLAGS) -DKERNEL_MAIN=$(KERNEL_MAIN) -c $< -o $@ -O2


############################################################################
#
# Compile "user provided" parts of OS
#
###########################################################################
# Headers shouldn't change much.  If they do, then just re-build all the 
# .o files
$(USR_OBJ)/%.o: $(USR_SRC)/%.c $(OS_SRC)/*.h $(wildcard $(USR_SRC)/*.h) | $(USR_OBJ)
	$(elfCC) $(CFLAGS) -c $< -o $@ -I $(OS_SRC)
        # generate the assembly file, in case we want to look at it later.
	$(elfCC) $(CFLAGS) -S $< -o $(@:.o=.s) -I $(OS_SRC) 


##########################################################################
#
# Prepare the grub config file
#
# This rule builds a custom grub config file that 
# includes the build time in the name of the image.
#
# Listing the build time in the grub menu allows us to 
# easily see at boot time that we are loading the 
# correct image.  
#
# (We found that we would occationally fail to notice
# a failure when running the makefile. As a result, we 
# would unknowning run an image that we mistakenly 
# believed had been updated.  Glancing at the build 
# time in the grub menu lets us verify that the image
# was correctly updated.)
########################################################################

# isodir contains the new OS image.  We place the  updated grub image 
# (the one with the current timestamp) in a "magic" location in this directory
# This target is declared to be .PHONY so that it is rebuilt every time.

.PHONY: isodir/boot/grub/grub.cfg
isodir/boot/grub/grub.cfg: | makefile.local
	echo $(MYOS_NAME)
	mkdir -p $(@D)
	m4 -DMYOS_NAME=$(MYOS_NAME) -DTIMESTAMP="`date`" grub.cfg.m4 > $@


#######################################################################
#
# Build the OS image
#
#######################################################################

# Create the binary image for the OS

# The filename "theos.bin" is used by boot/grub/grub.cfg.  
# You can change the name of the bin file to mach the name of the iso file, 
# just be sure to update grub.cfg also if you do.
isodir/boot/theos.bin: $(all_objs)
	mkdir -p $(@D)
	$(elfCC) -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc



# Create the bootable iso image
timingos.iso: isodir/boot/theos.bin isodir/boot/grub/grub.cfg
	grub2-mkrescue -o $@ isodir



# Create an executable that can be run locally for debugging (checking 
# output, using valgrind, etc.)
timingos.debug: $(os_sources) $(usr_sources) 
	$(CC) -DDEBUG -g -std=c99 -o $@ $^ -I $(OS_SRC)


#######################################################################
#
# clean
#
#######################################################################

clean::
	rm obj/*/* timingos.iso timingos.debug isodir/boot/theos.bin
