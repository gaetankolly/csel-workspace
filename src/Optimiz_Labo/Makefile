DIRS=$(filter-out Makefile, $(wildcard *))

all clean install clean_all:
	for dir in $(DIRS); do $(MAKE) $@ -C $$dir; done

