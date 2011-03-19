CSRC := $(shell find src -name "*.c")
CHDR := $(shell find include -name "*.h")

COBJ := $(CSRC:.c=.o)

CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c99 -Iinclude/
LNFLAGS := 
EXE := inari

###

all: $(BUILD_DIR) $(COBJ) $(EXE)

$(EXE): 
	@ echo "  LINK" $(EXE)
	@ $(CC) $(CFLAGS) $(LNFLAGS) $(COBJ) -o $(EXE)

%.o: %.c
	@ echo "  CC" $<
	@ $(CC) $(CFLAGS) -c $< -o $@

debug:
	@$(MAKE) "CFLAGS=$(CFLAGS) -g"

gcc:
	@$(MAKE) "CC=gcc"

clang:
	@$(MAKE) "CC=clang"

clean:
	rm -f $(COBJ) $(EXE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY=all clean debug gcc
