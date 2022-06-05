EXE=example-gprof
SRCS=$(wildcard *.c) 

ifeq ($(target),)
target=nano
endif

CFLAGS=-Wall -Wextra -g -c -O1 -MD -std=gnu11 -D_GNU_SOURCE -pg

ifeq ($(target),nano)
TOOLCHAIN_PATH=/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)aarch64-linux-
CFLAGS+=-mcpu=cortex-a53 -funwind-tables
CFLAGS+=-O2 -fno-omit-frame-pointer
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
OBJDUMP=$(TOOLCHAIN)objdump

OBJDIR=.obj/$(target)
OBJS= $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

$(OBJDIR)/%o: %c
	$(CC) $(CFLAGS) $< -o $@
	
all: $(OBJDIR)/ $(EXEC)
	
$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/:
	mkdir -p $(OBJDIR)

clean:
	rm -Rf $(OBJDIR) $(EXEC) $(EXEC)_s *~ t.txt

clean_all: clean
	rm -Rf .obj $(EXE) $(EXE)_s $(EXE)_a $(EXE)_a_s $(EXE)_h $(EXE)_h_s
	@rm -f perf.data perf.data.old gmon.out gprof.out

dump: all
	$(OBJDUMP) -dS $(EXEC) > t.txt

gprof-generate: all
	./$(EXECUTABLE)
	gprof $(EXECUTABLE) > gprof.out

gprof-read:
	less gprof.out

-include $(OBJS:.o=.d)

.PHONY: all clean clean_all dump
