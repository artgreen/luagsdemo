#ifndef LUAGS_H
#define LUAGS_H
#include "lua.h"
#define LG_MAX_SCRIPTS 8
#define LG_PATH_SIZE 64
/* One state, owned by this interface. Zero means success throughout. */
int lg_open(void);
void lg_close(void);
int lg_initialize(lua_CFunction setup);
int lg_run_file(const char *name);
int lg_run_string(const char *code);
int lg_get_scripts(char files[][LG_PATH_SIZE], int *count);
int lg_get_integer(const char *name, lua_Integer *value);
int lg_call_integer(const char *name, lua_Integer arg, lua_Integer *result);
#endif
