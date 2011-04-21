/*
    Copyright (C) 2010-2011 wpitchoune@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#ifndef _P_LUATPL_H
#define _P_LUATPL_H

#include <lua.h>

#define LUATPL_ERROR_LUA_FILE_LOAD 1
#define LUATPL_ERROR_INIT 2
#define LUATPL_ERROR_LUA_EXECUTION 3
#define LUATPL_ERROR_WRONG_RETURN_TYPE 4
#define LUATPL_ERROR_LUA_STATE_OPEN 5

struct luatpl_error {
	unsigned int code;

	char *message;
};

/*
  Generates a string which is the result of a Lua script execution.

  The string is retrieved from the top element of the Lua stack
  after the Lua script execution.

  If not 'NULL' the 'init' function is called after Lua environment
  setup and before Lua script execution. This function typically puts
  input information for the Lua script into the stack.

  'init_data' is passed to the second parameter of 'init' function

  Returns the generated string on success, or NULL on error.
 */
char *luatpl_generate(const char *lua,
		      int (*init) (lua_State *, void *),
		      void *init_data,
		      struct luatpl_error *err);

/*
  Generates a file which is the result of a Lua script execution.

  See luatpl_generate function for 'init', 'init_data', and 'err'
  parameters.

  'dst_path' is the path of the generated file

  Returns '1' on success, or '0' on error.

 */
int luatpl_generate_file(const char *lua,
			 int (*init) (lua_State *, void *),
			 void *init_data,
			 const char *dst_path,
			 struct luatpl_error *err);

void luatpl_fprint_error(FILE *stream,
			 const struct luatpl_error *err,
			 const char *lua,
			 const char *dst_path);

#endif
