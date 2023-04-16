#
# Crappy Makefile
#
EXE_DIR := ../build
LIB_DIR := lib
names := main
SRCS := $(names:%=*.c)
OBJS := $(foreach wrd, $(names),$(wrd).a)
ADDS := $(foreach wrd, $(names),+$(wrd).a)
CFLAGS := -I -P -D

.PHONY: all fixlib
all: luademo

luademo: $(OBJS)
	iix link $(OBJS) $(LIB_DIR)/lua.lib KEEP=$@

fixlib:
	iix chtype -t lib $(LIB_DIR)/lua.lib
clean:
	@rm -f -- *.sym *.a *.b *.d *.e *.root

# Dependencies
main.a		: main.c

%.a:
	iix compile $(CFLAGS) $<
