bin_PROGRAMS =  psensor-server
psensor_server_SOURCES = server.c server.h

AM_CPPFLAGS = -Wall -Werror -DDEFAULT_WWW_DIR=\""$(pkgdatadir)/www"\"\
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	$(SENSORS_CFLAGS)\
	$(JSON_CFLAGS)\
	$(LIBMICROHTTPD_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = \
	../lib/libpsensor.a \
	$(SENSORS_LIBS) \
	$(JSON_LIBS) \
	$(LIBMICROHTTPD_LIBS) \
	$(PTHREAD_LIBS)

if GTOP
AM_CPPFLAGS += $(GTOP_CFLAGS)
LIBS += $(GTOP_LIBS) 
AM_LDFLAGS = -Wl,--as-needed
psensor_server_SOURCES += sysinfo.h sysinfo.c
endif

if ATASMART
AM_CPPFLAGS += $(ATASMART_CFLAGS)
LIBS += $(ATASMART_LIBS)
endif

if HELP2MAN
psensor-server.1: server.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) psensor-server$(EXEEXT)
	help2man --include=description.txt -N --name="Temperature and system monitoring Web server" --output=psensor-server.1 ./psensor-server$(EXEEXT)
endif

dist_man_MANS =  psensor-server.1
EXTRA_DIST = description.txt
