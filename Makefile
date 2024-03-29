#
# Crappy Makefile
#

# apple commander
AC := ac
# nulib2 for shrinkit
NULIB := nulib2

EXE_DIR := ../build
LIB_DIR := lib
RES_DIR := resources
DEMO_DISK := $(RES_DIR)/luags-demo.po
DEMO_VOL := LUAGSDEMO

names := main collection luacollection luastatus luafuncs luags
SRCS := $(names:%=*.c)
OBJS := $(foreach wrd, $(names),$(wrd).a)
ADDS := $(foreach wrd, $(names),+$(wrd).a)
CFLAGS := -I -P -D

.PHONY: all fixlib relink
all: luademo demodisk relink
relink:
	iix link $(OBJS) $(LIB_DIR)/lvm $(LIB_DIR)/lua.lib KEEP=luademo || rm -f -- $@  # we need to delete the exe on error
	iix luademo

run : clean luademo
	iix luademo

luademo: $(OBJS)
	iix link $(OBJS) $(LIB_DIR)/lvm $(LIB_DIR)/lua.lib KEEP=$@ || rm -f -- $@  # we need to delete the exe on error
	iix chtyp -t exe $@
	cp $@ $(RES_DIR)

demodisk: luademo stattest.lua coltest.lua | $(RES_DIR)
	$(AC) -pro800 $(DEMO_DISK) $(DEMO_VOL)
	<luademo $(AC) -p $(DEMO_DISK) luademo exe
	<luademo $(AC) -p $(DEMO_DISK) luademos16 s16
	<stattest.lua $(AC) -ptx $(DEMO_DISK) stattest.lua
	<coltest.lua $(AC) -ptx $(DEMO_DISK) coltest.lua
	<config.lua $(AC) -ptx $(DEMO_DISK) config.lua
	$(AC) -l $(DEMO_DISK)

fixlib:
	iix chtyp -t lib $(LIB_DIR)/lua.lib
	iix chtyp -t obj $(LIB_DIR)/lvm.a
clean:
	@rm -f -- *.sym *.a *.b *.d *.e *.root $(DEMO_DISK)

$(RES_DIR):
	mkdir -p $(RES_DIR)

# Dependencies
main.a				: main.c
collection.a		: collection.c collection.h
luacollection.a		: luacollection.c
luastatus.a			: luastatus.c
luafuncs.a			: luafuncs.c luafuncs.h
luags.a				: luags.c luags.h
stattest.lua:
	touch $@
coltest.lua:
	touch $@

%.a:
	iix compile $(CFLAGS) $<
