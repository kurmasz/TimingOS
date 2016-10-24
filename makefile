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

MYOS_NAME = "My Test OS"

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


# This rule builds the obj directory. That needs to be done only when
# cloning the repository, so I don't make this a depencency of each 
# compilation.
directories: 
	mkdir -p $(OS_OBJ)
	mkdir -p $(USR_OBJ)

#############################################################################
#
# Compile "core" OS
#
#############################################################################
# Headers shouldn't change much.  If they do, then just re-build all the 
# .o files
$(OS_OBJ)/%.o: $(OS_SRC)/%.c $(OS_SRC)/*.h
	$(elfCC) $(CFLAGS) -c $< -o $@ -O2

$(OS_OBJ)/boot.o: $(OS_SRC)/boot.s
	$(elfCC) $(CFLAGS) -c $< -o $@ -O2


############################################################################
#
# Compile "user provided" parts of OS
#
###########################################################################
# Headers shouldn't change much.  If they do, then just re-build all the 
# .o files
$(USR_OBJ)/%.o: $(USR_SRC)/%.c  $(OS_SRC)/*.h $(wildcard $(USR_SRC)/*.h)
	$(elfCC) $(CFLAGS) -c $< -o $@ -I $(OS_SRC)



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

# isodir contains the new OS image.  We place the 
# updated grub image (the one with the current timestamp)
# in a "magic" location in this directory

isodir/boot/grub/grub.cfg::
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
