SUBDIRS = lib glade

# -export-dynamic is need for defining handlers in the glade file.
AM_LDFLAGS = -Wl,--as-needed -export-dynamic

if LIBMICROHTTPD
if JSON
SUBDIRS += server
endif
endif

# -no-deprecated-declarations to avoid some glib deprecation warnings
AM_CPPFLAGS = -Wno-deprecated-declarations -Wall -Werror -DDEFAULT_WWW_DIR=\""$(pkgdatadir)/www"\"\
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/unity \
	$(GTK_CFLAGS)\
	$(GCONF_CFLAGS)\
	$(SENSORS_CFLAGS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = \
	lib/libpsensor.a \
	$(GTK_LIBS)\
	$(GCONF_LIBS)\
	$(PTHREAD_LIBS)\
	$(SENSORS_LIBS)

if GTK
if X11
if XEXT
if GCONF
bin_PROGRAMS = psensor
dist_man_MANS = psensor.1
endif
endif
endif
endif

psensor_SOURCES = \
	cfg.h cfg.c \
	graph.h graph.c \
	main.c \
	ui.h ui.c \
	ui_appindicator.h \
	ui_color.h ui_color.c \
	ui_graph.h ui_graph.c \
	ui_pref.h ui_pref.c \
	ui_sensorlist.h ui_sensorlist.c \
	ui_sensorpref.h ui_sensorpref.c \
	ui_status.h ui_status.c

if GTOP
AM_CPPFLAGS += $(GTOP_CFLAGS)
LIBS += $(GTOP_LIBS)
endif

if LIBNOTIFY
psensor_SOURCES += ui_notify.h ui_notify.c
LIBS += $(LIBNOTIFY_LIBS)
AM_CPPFLAGS += $(LIBNOTIFY_CFLAGS)
endif

if APPINDICATOR 
psensor_SOURCES += ui_appindicator.c 
LIBS +=	$(APPINDICATOR_LIBS)
AM_CPPFLAGS += $(APPINDICATOR_CFLAGS)
endif

if NVIDIA
if X11
AM_CPPFLAGS += $(NVIDIA_CFLAGS)
LIBS +=  $(NVIDIA_LIBS)
endif
endif

if LIBATIADL
AM_CPPFLAGS += $(LIBATIADL_CFLAGS)
LIBS += $(LIBATIADL_LIBS)
endif

if ATASMART
AM_CPPFLAGS += $(ATASMART_CFLAGS)
LIBS += $(ATASMART_LIBS)
endif

if UNITY
psensor_SOURCES += ui_unity.h ui_unity.c
AM_CPPFLAGS += $(UNITY_CFLAGS)
LIBS += $(UNITY_LIBS) 
endif

if CURL
if JSON
psensor_SOURCES += rsensor.h rsensor.c
LIBS += $(CURL_LIBS) $(JSON_LIBS)
AM_CPPFLAGS += $(CURL_CFLAGS) $(JSON_CFLAGS)
endif
endif

EXTRA_DIST = description.txt psensor.schemas

if GCONF
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = psensor.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/src/$(schema_DATA)
endif

psensor.1: main.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) psensor$(EXEEXT)
	help2man --include=description.txt -N --name="Temperature monitoring application" --output=psensor.1 ./psensor$(EXEEXT)

