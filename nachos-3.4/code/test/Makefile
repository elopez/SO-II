# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

.PHONY: clean all

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
GCCDIR = `pwd`/mips-dec-ultrix42/mips-
LDFLAGS = -T script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = $(GCCDIR)cpp
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -mips1

BINS = halt shell matmult sort filetest printargs busy cp cat ortest tlbstress

all: lib/gcc-lib $(BINS)

lib/gcc-lib:
	ln -s `pwd`/mips-dec-ultrix42/ lib/gcc-lib

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

# Estas reglas sirven para compilar programas simples,
# que consistan en un único fuente.
# Las reglas anteriores para construir los ejecutables
# halt, shell, sort y filetest se podrían suprimir

# Override a la regla builtin de make
%: %.c

# make provee una regla .c -> .o

# .o -> COFF
%.coff: start.o %.o minilib.o
	$(LD) $(LDFLAGS) $^ -o $@

# COFF -> NOFF
%: %.coff
	../bin/coff2noff $< $@

# Regla para limpiar
clean:
	rm -f $(BINS) *.o *.coff
