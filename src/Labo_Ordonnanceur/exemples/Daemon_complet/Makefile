EXE=daemon
SRCS=$(wildcard *.c)

ifeq ($(target),)
target=nano
endif

CFLAGS=-Wall -Wextra -g -c -O0 -MD -std=gnu11

ifeq ($(target),nano)
TOOLCHAIN_PATH=~/workspace/nano/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)aarch64-linux-gnu-
CFLAGS+=-mcpu=cortex-a53 -funwind-tables
##CFLAGS+=-O2 -fno-omit-frame-pointer
OBJDIR=.obj/nano
EXEC=$(EXE)
endif

ifeq ($(target),host)
EXEC=$(EXE)_h
endif

ifeq ($(target),xu3)
TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-gnueabihf-
CFLAGS+=-mcpu=cortex-a15.cortex-a7 -funwind-tables
##CFLAGS+=-O2 -fno-omit-frame-pointer
OBJDIR=.obj/xu3
EXEC=$(EXE)_a
endif

CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip

OBJDIR=.obj/$(target)
OBJS= $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@

all: $(OBJDIR)/ $(EXEC)
	echo $(EXEC)
	echo $(SRCS)

$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/:
	mkdir -p $(OBJDIR)

clean:
	rm -Rf $(OBJDIR) $(EXEC) $(EXEC)_s *~

clean_all: clean
	rm -Rf .obj $(EXE) $(EXE)_s $(EXE)_a $(EXE)_a_s $(EXE)_h $(EXE)_h_s

-include $(OBJS:.o=.d)

.PHONY: all clean clean_all
