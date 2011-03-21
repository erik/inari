CSRC := $(shell find . -type f -name "*.c")
CHDR := $(shell find . -type f -name "*.h")

COBJ := $(CSRC:.c=.o)

PLUGINS := $(CSRC:.c=.so)

CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c99 -I../include/ -fPIC
LNFLAGS := -shared

all: $(COBJ) $(PLUGINS)

clean:
	rm -f $(COBJ)
	rm -f $(PLUGINS)

debug:
	@$(MAKE) "CFLAGS=$(CFLAGS) -g -O0"

%.o: %.c
	@ echo "   CC " $<
	@ $(CC) $(CFLAGS) -c $< -o $@

%.so: %.o
	@ echo "   LINK " $<
	@ ld $< $(LNFLAGS) -soname $@ -o $@

.PHONY=clean debug
