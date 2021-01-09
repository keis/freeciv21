/**************************************************************************
 Copyright (c) 1996-2020 Freeciv21 and Freeciv contributors. This file is
 part of Freeciv21. Freeciv21 is free software: you can redistribute it
 and/or modify it under the terms of the GNU  General Public License  as
 published by the Free Software Foundation, either version 3 of the
 License,  or (at your option) any later version. You should have received
 a copy of the GNU General Public License along with Freeciv21. If not,
 see https://www.gnu.org/licenses/.
**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <fc_config.h>
#endif

/* common/scriptcore */
#include "luascript.h"

/* server/scripting */
#include "script_fcdb.h"

#include "api_fcdb_base.h"

/**
   Return the value for the fcdb setting 'type'.
 */
const char *api_fcdb_option(lua_State *L, const char *type)
{
  LUASCRIPT_CHECK_STATE(L, NULL);
  LUASCRIPT_CHECK_ARG_NIL(L, type, 2, string, NULL);

  return fcdb_option_get(type);
}
