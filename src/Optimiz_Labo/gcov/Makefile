CC?=gcc
CFLAGS=-std=c11 -Wall -g -fprofile-arcs -ftest-coverage
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=example-gcov

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
	@rm -f *.gcno *.gcda *.gcov


gcov-generate: all
	./$(EXECUTABLE)
	gcov main.c

gcov-read: gcov-generate
	less main.c.gcov


-include *.d
