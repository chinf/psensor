SUBDIRS = lib glade

psensor_SOURCES = \
	cfg.h cfg.c \
	notify_cmd.c notify_cmd.h \
	graph.h graph.c \
	main.c \
	pxdg.h pxdg.c \
	rsensor.h \
	ui.h ui.c \
	ui_appindicator.h \
	ui_color.h ui_color.c \
	ui_graph.h ui_graph.c \
	ui_notify.h \
	ui_pref.h ui_pref.c \
	ui_sensorlist.h ui_sensorlist.c \
	ui_sensorpref.h ui_sensorpref.c \
	ui_status.h ui_status.c \
	ui_unity.h

# -export-dynamic is need for defining handlers in the glade file.
AM_LDFLAGS = -Wl,--as-needed -export-dynamic

if LIBMICROHTTPD
if JSON
SUBDIRS += server
endif
endif

AM_CPPFLAGS =-Wall \
	-DDEFAULT_WWW_DIR=\""$(pkgdatadir)/www"\"\
	-DDATADIR=\""$(datadir)"\"\
	-DPSENSOR_DESKTOP_FILE=\""psensor.desktop"\"\
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/unity \
	$(GTK_CFLAGS)\
	$(SENSORS_CFLAGS)

DEFS = -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = \
	lib/libpsensor.a \
	$(GTK_LIBS)\
	$(PTHREAD_LIBS)\
	$(SENSORS_LIBS) -lm

if GTK
if X11
if XEXT
bin_PROGRAMS = psensor
dist_man_MANS = psensor.1
endif
endif
endif

if GTOP
AM_CPPFLAGS += $(GTOP_CFLAGS)
LIBS += $(GTOP_LIBS)
endif

if LIBNOTIFY
psensor_SOURCES += ui_notify.c
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

if LIBUDISKS2
LIBS += $(LIBUDISKS2_LIBS)
endif

if UNITY
psensor_SOURCES += ui_unity.c
AM_CPPFLAGS += $(UNITY_CFLAGS)
LIBS += $(UNITY_LIBS) 
endif

if CURL
if JSON
psensor_SOURCES += rsensor.c
LIBS += $(CURL_LIBS) $(JSON_LIBS)
AM_CPPFLAGS += $(CURL_CFLAGS) $(JSON_CFLAGS)
endif
endif

if HELP2MAN
psensor.1: main.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) psensor$(EXEEXT)
	help2man --include=description.txt -N --name="Temperature monitoring application" --output=psensor.1 ./psensor$(EXEEXT)
endif

gsettings_SCHEMAS=psensor.gschema.xml

EXTRA_DIST = description.txt\
	$(gsettings_SCHEMAS)\
	rsensor.c\
	ui_appindicator.c\
	ui_notify.c\
	ui_unity.c

@GSETTINGS_RULES@
