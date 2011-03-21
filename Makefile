CSRC := $(shell find src -name "*.c")
CHDR := $(shell find include -name "*.h")

COBJ := $(CSRC:.c=.o)

CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c99 -Iinclude/
LNFLAGS := -ldl -rdynamic
EXE := inari

###

all: $(COBJ) $(EXE) plugs install

install:
	@ mkdir -p ~/.inari
	@ echo "Copying .inari => ~/.inari/"
	@ cp -r .inari/* ~/.inari/

plugs:
	@ mkdir -p .inari/native
	@ cd plugins; $(MAKE) all
	@ cp plugins/*.so .inari/native/

$(EXE): $(COBJ)
	@ echo "  LINK" $(EXE)
	@ $(CC) $(CFLAGS) $(LNFLAGS) $(COBJ) -o $(EXE)

%.o: %.c
	@ echo "  CC" $<
	@ $(CC) $(CFLAGS) -c $< -o $@

debug:
	@ cd plugins; make debug
	@$(MAKE) "CFLAGS=$(CFLAGS) -g -O0"

gcc:
	@$(MAKE) "CC=gcc"

clang:
	@$(MAKE) "CC=clang"

clean:
	rm -f $(COBJ) $(EXE)
	@ cd plugins; make clean

.PHONY=all clean debug gcc
