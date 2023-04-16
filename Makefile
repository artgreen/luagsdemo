#
# Crappy Makefile
#
EXE_DIR := ../build

names := main
SRCS := $(names:%=*.c)
OBJS := $(foreach wrd, $(names),$(wrd).a)
ADDS := $(foreach wrd, $(names),+$(wrd).a)
CFLAGS := -I -P -D

.PHONY: all
all: luademo

luademo: $(OBJS)
	iix link $(OBJS) KEEP=$@

clean:
	@rm -f -- *.sym *.a *.b *.d *.e *.root lua luac *.lib

# Dependencies
main.a		: main.c

%.a:
	iix compile $(CFLAGS) $<
