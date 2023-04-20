
#ifndef LUAGSDEMO_LUAGS_H
#define LUAGSDEMO_LUAGS_H

#include "lstate.h"

int lg_open(void);
void lg_close(void);
lua_State *lg_state(void);
void lg_openlibs(void);
const char *lg_run_file(char *);
void lg_load_module(void (*module_func)(lua_State *));
void get_array(lua_State *L, char *name);

#endif //LUAGSDEMO_LUAGS_H
