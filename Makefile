CSRC := $(shell find src -name "*.c")
CHDR := $(shell find include -name "*.h")

COBJ := $(CSRC:.c=.o)

LUA := lua-5.1.4
LIBLUA := $(LUA)/lib/liblua.a

CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c99 -Iinclude/ -I$(LUA)/include
LNFLAGS :=  -ldl -lm -rdynamic $(LIBLUA) 
EXE := inari

###

all: $(LUA) $(COBJ) $(EXE) plugs install

$(LUA):
	@ wget "http://www.lua.org/ftp/$(LUA).tar.gz"
	@ tar -xvf $(LUA).tar.gz
	@ rm $(LUA).tar.gz
	@ echo "Building Lua."
	@ cd $(LUA); make linux local

install:
	@ mkdir -p ~/.inari
	@ echo "Copying .inari => ~/.inari/"
	@ cp -r .inari/* ~/.inari/

plugs:
	@ mkdir -p .inari/native .inari/lua
	@ cd plugins; $(MAKE) all
	@ cp plugins/*.so .inari/native/
	@ cp plugins/lua/*.lua .inari/lua/

$(EXE): $(COBJ)
	@ echo "  LINK" $(EXE)
	@ $(CC) $(COBJ) $(LNFLAGS) -o $(EXE)

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
