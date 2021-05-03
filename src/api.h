#ifndef _api_h_
#define _api_h_

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <sglib.h>

int clua_init();
int clua_load_modules();
int clua_close();

#endif
