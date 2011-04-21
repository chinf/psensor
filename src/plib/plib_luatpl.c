/*
    Copyright (C) 2010-2011 jeanfi@gmail.com

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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdlib.h>
#include <string.h>

#include <lualib.h>
#include <lauxlib.h>

#include "plib_luatpl.h"

char *
luatpl_generate(const char *lua,
		int (*init) (lua_State *, void *),
		void *init_data,
		struct luatpl_error *err)
{
	lua_State *L;
	char *page = NULL;

	L = lua_open();
	if (!L) {
		err->code = LUATPL_ERROR_LUA_STATE_OPEN;
		return NULL;
	}

	luaL_openlibs(L);

	if (!init || init(L, init_data)) {

		if (!luaL_loadfile(L, lua)) {
			if (!lua_pcall(L, 0, 1, 0)) {
				if (lua_isstring(L, -1))
					page = strdup(lua_tostring(L, -1));
				else
					err->code =
						LUATPL_ERROR_WRONG_RETURN_TYPE;
			} else {
				err->code = LUATPL_ERROR_LUA_EXECUTION;
				err->message = strdup(lua_tostring(L, -1));
			}
		} else {
			err->code = LUATPL_ERROR_LUA_FILE_LOAD;
		}
	} else {
		err->code = LUATPL_ERROR_INIT;
	}

	lua_close(L);

	return page;
}

int
luatpl_generate_file(const char *lua,
		     int (*init) (lua_State *, void *),
		     void *init_data,
		     const char *dst_path,
		     struct luatpl_error *err)
{
	FILE *f;
	int ret;
	char *content;

	ret = 1;

	content = luatpl_generate(lua, init, init_data, err);

	if (content) {
		f = fopen(dst_path, "w");

		if (f) {
			if (fputs(content, f) == EOF)
				ret = 0;

			if (fclose(f) == EOF)
				ret = 0;
		} else {
			ret = 0;
		}

		free(content);
	} else {
		ret = 0;
	}

	return ret;
}

void
luatpl_fprint_error(FILE *stream,
		    const struct luatpl_error *err,
		    const char *lua,
		    const char *dst_path)
{
	if (!err || !err->code)
		return ;

	switch (err->code) {
	case LUATPL_ERROR_LUA_FILE_LOAD:
		fprintf(stream,
			_("LUATPL Error: failed to load Lua script: %s.\n"),
			lua);
		break;

	case LUATPL_ERROR_INIT:
		fprintf(stream,
			_("LUATPL Error: failed to call init function: %s.\n"),
			lua);
		break;

	case LUATPL_ERROR_LUA_EXECUTION:
		fprintf(stream,
			_("LUATPL Error:"
			  "failed to execute Lua script (%s): %s.\n"),
			lua, err->message);
		break;

	case LUATPL_ERROR_WRONG_RETURN_TYPE:
		fprintf(stream,
			_("LUATPL Error:"
			  "lua script (%s) returned a wrong type.\n"),
			lua);
		break;

	case LUATPL_ERROR_LUA_STATE_OPEN:
		fprintf(stream,
			_("LUATPL Error:"
			  "failed to open lua state.\n"));
		break;


	default:
		fprintf(stream,
			_("LUATPL Error: code: %d.\n"),
			err->code);
	}
}
