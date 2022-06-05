CC?=gcc
CFLAGS=-std=c11 -Wall -pg
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=example-gprof

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@


clean:
	@rm -f $(OBJECTS)
	@rm -f *.d

clean_all: clean
	@rm -f $(EXECUTABLE)
	@rm -f perf.data perf.data.old
	@rm -f gmon.out gprof.out


gprof-generate: all
	./$(EXECUTABLE)
	gprof $(EXECUTABLE) > gprof.out

gprof-read:
	less gprof.out


-include *.d
