
#ifndef LUAGSDEMO_LUAGS_H
#define LUAGSDEMO_LUAGS_H

#include "lstate.h"

int lg_open(void);
void lg_close(void);
lua_State *lg_state(void);
void lg_openlibs(void);
const char *lg_run_file(const char *file_name);
void lg_load_module(void (*module_func)(lua_State *));
int lg_get_string_array(char *name, const char **array);

#endif //LUAGSDEMO_LUAGS_H
