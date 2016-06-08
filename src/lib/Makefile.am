noinst_LIBRARIES = libpsensor.a

libpsensor_a_SOURCES = \
	amd.h\
	bool.h\
	color.h color.c\
	hdd.h hdd_hddtemp.c\
	lmsensor.h\
	measure.h measure.c\
	nvidia.h\
	parray.h\
	pgtop2.h\
	plog.h plog.c\
	pmutex.h pmutex.c\
	psensor.h psensor.c\
	ptime.h ptime.c\
	pio.h pio.c\
	pudisks2.h\
	slog.c slog.h\
	temperature.c temperature.h\
	url.c url.h

AM_CPPFLAGS = -Wall -Werror

if SENSORS
libpsensor_a_SOURCES += lmsensor.c
LIBS += $(SENSORS_LIBS)
endif

if ATASMART
libpsensor_a_SOURCES += hdd_atasmart.c
LIBS += $(ATASMART_LIBS)
AM_CPPFLAGS += $(ATASMART_CFLAGS)
endif

if NVIDIA
libpsensor_a_SOURCES += nvidia.c
endif

if LIBATIADL
LIBS += $(LIBATIADL_LIBS)
AM_CPPFLAGS += $(LIBATIADL_CFLAGS)
libpsensor_a_SOURCES += amd.c
endif

if GTOP
libpsensor_a_SOURCES += pgtop2.c
LIBS += $(GTOP_LIBS)
AM_CPPFLAGS += $(GTOP_CFLAGS)
endif

if JSON
libpsensor_a_SOURCES += psensor_json.h psensor_json.c
LIBS += $(JSON_LIBS)
AM_CPPFLAGS += $(JSON_CFLAGS)
endif

if LIBUDISKS2
libpsensor_a_SOURCES += pudisks2.c
LIBS += $(LIBUDISKS2_LIBS)
AM_CPPFLAGS += $(LIBUDISKS2_CFLAGS)
endif

EXTRA_DIST=$(libpsensor_a_SOURCES) \
	amd.c \
	pgtop2.c \
	lmsensor.c \
	nvidia.c \
	psensor_json.h psensor_json.c \
	pudisks2.c
