#
# Crappy Makefile
#
EXE_DIR := ../build
LIB_DIR := lib

names := main collection lua_collection
SRCS := $(names:%=*.c)
OBJS := $(foreach wrd, $(names),$(wrd).a)
ADDS := $(foreach wrd, $(names),+$(wrd).a)
CFLAGS := -I -P -D

.PHONY: all fixlib
all: luademo

luademo: $(OBJS)
	iix link $(OBJS) $(LIB_DIR)/lvm $(LIB_DIR)/lua.lib KEEP=$@ || rm -f -- $@  # we need to delete the exe on error

fixlib:
	iix chtyp -t lib $(LIB_DIR)/lua.lib
	iix chtyp -t obj $(LIB_DIR)/lvm.a
clean:
	@rm -f -- *.sym *.a *.b *.d *.e *.root

# Dependencies
main.a				: main.c
collection.a		: collection.c collection.h
lua_collection.a	: lua_collection.c

%.a:
	iix compile $(CFLAGS) $<
