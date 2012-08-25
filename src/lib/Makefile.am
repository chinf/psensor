noinst_LIBRARIES = libpsensor.a

libpsensor_a_SOURCES = \
	amd.h \
	bool.h \
	color.h color.c \
	cpu.h \
	hdd.h hdd_hddtemp.c\
	lmsensor.h lmsensor.c \
	log.h log.c \
	measure.h measure.c \
	nvidia.h \
	psensor.h psensor.c\
	pio.h pio.c \
	slog.c slog.h \
	url.c url.h

AM_CPPFLAGS = -Wall -Werror

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
libpsensor_a_SOURCES += cpu.c
LIBS += $(GTOP_LIBS)
AM_CPPFLAGS += $(GTOP_CFLAGS)
endif

if JSON
libpsensor_a_SOURCES += psensor_json.h psensor_json.c
LIBS += $(JSON_LIBS)
AM_CPPFLAGS += $(JSON_CFLAGS)
endif

EXTRA_DIST=$(libpsensor_a_SOURCES) \
	amd.c \
	cpu.c \
	nvidia.c