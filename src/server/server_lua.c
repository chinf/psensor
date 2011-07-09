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

#include <stdlib.h>
#include <string.h>

#include "server_lua.h"

#include "plib/plib_luatpl.h"

int init_lua(lua_State *L, void *data)
{
	struct server_data *server_data = data;
	struct psensor **s_cur;
	struct psensor **sensors = server_data->sensors;
	int i;
	static float load_scale = 1 << SI_LOAD_SHIFT;

	lua_newtable(L);

#ifdef HAVE_GTOP
	lua_pushstring(L, "load");
	lua_pushnumber(L, server_data->psysinfo.cpu_rate);
	lua_settable(L, -3);
#endif

	lua_pushstring(L, "uptime");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.uptime);
	lua_settable(L, -3);

	lua_pushstring(L, "load_1mn");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.loads[0] / load_scale);
	lua_settable(L, -3);

	lua_pushstring(L, "load_5mn");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.loads[1] / load_scale);
	lua_settable(L, -3);

	lua_pushstring(L, "load_15mn");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.loads[2] / load_scale);
	lua_settable(L, -3);

	lua_pushstring(L, "freeram");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.freeram);
	lua_settable(L, -3);

	lua_pushstring(L, "sharedram");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.sharedram);
	lua_settable(L, -3);

	lua_pushstring(L, "bufferram");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.bufferram);
	lua_settable(L, -3);

	lua_pushstring(L, "totalswap");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.totalswap);
	lua_settable(L, -3);

	lua_pushstring(L, "freeswap");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.freeswap);
	lua_settable(L, -3);

	lua_pushstring(L, "procs");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.procs);
	lua_settable(L, -3);

	lua_pushstring(L, "totalhigh");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.totalhigh);
	lua_settable(L, -3);

	lua_pushstring(L, "freehigh");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.freehigh);
	lua_settable(L, -3);

	lua_pushstring(L, "totalram");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.totalram);
	lua_settable(L, -3);

	lua_pushstring(L, "mem_unit");
	lua_pushnumber(L, server_data->psysinfo.sysinfo.mem_unit);
	lua_settable(L, -3);

	lua_setglobal(L, "sysinfo");

	lua_newtable(L);

	s_cur = sensors;
	i = 1;
	while (*s_cur) {
		lua_pushnumber(L, i);

		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, (*s_cur)->name);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_last");
		lua_pushnumber(L, psensor_get_current_value(*s_cur));
		lua_settable(L, -3);

		lua_pushstring(L, "measure_min");
		lua_pushnumber(L, (*s_cur)->min);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_max");
		lua_pushnumber(L, (*s_cur)->max);
		lua_settable(L, -3);

		lua_settable(L, -3);

		s_cur++;
		i++;
	}

	lua_setglobal(L, "sensors");

	lua_pushstring(L, VERSION);
	lua_setglobal(L, "psensor_version");

	return 1;
}

char *lua_to_html_page(struct server_data *server_data, const char *fpath)
{
	char *page = NULL;
	struct luatpl_error err;

	err.message = NULL;

	page = luatpl_generate(fpath,
			       init_lua,
			       server_data,
			       &err);

	if (!page) {
		luatpl_fprint_error(stderr,
				    &err,
				    fpath,
				    "outstring");
		free(err.message);
	}

	return page;
}
