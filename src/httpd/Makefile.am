AM_CPPFLAGS = -pedantic -Werror -DDEFAULT_WWW_DIR=\""$(pkgdatadir)/www"\"\
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	$(LUA_CFLAGS)\
	$(GTK_CFLAGS)\
	$(APPINDICATOR_CFLAGS)\
	$(GCONF_CFLAGS)\
	$(SENSORS_CFLAGS)\
	$(NVIDIA_CFLAGS)\
	$(CURL_CFLAGS)\
	$(JSON_CFLAGS)\
	$(GTOP_CFLAGS)\
	$(LIBMICROHTTPD_CFLAGS)

LIBS = \
	$(top_srcdir)/src/plib/libplib.a \
	$(top_srcdir)/src/lib/libpsensor.a \
	$(LUA_LIBS)\
	$(GTK_LIBS)\
	$(APPINDICATOR_LIBS)\
	$(GCONF_LIBS)\
	$(SENSORS_LIBS)\
	$(NVIDIA_LIBS)\
	$(CURL_LIBS)\
	$(JSON_LIBS)\
	$(GTOP_LIBS)\
	$(LIBMICROHTTPD_LIBS)

bin_PROGRAMS =  psensor-server

psensor_server_SOURCES = httpd.c httpd.h




dist_man_MANS =  psensor-server.1


psensor-server.1: httpd.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) psensor-server$(EXEEXT)
	help2man --output=psensor-server.1 ./psensor-server$(EXEEXT)