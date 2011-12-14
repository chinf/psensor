noinst_LIBRARIES = libpsensor.a

libpsensor_a_CFLAGS = -Wall -Werror

libpsensor_a_SOURCES = \
	amd.h \
	cpu.h \
	measure.h measure.c \
	color.h color.c \
	log.h log.c \
	psensor.h psensor.c\
	hdd.h\
	nvidia.h \
	lmsensor.h lmsensor.c \
	pio.h pio.c \
	url.c url.h

AM_CPPFLAGS=

if ATASMART
libpsensor_a_SOURCES += hdd_atasmart.c
LIBS += $(ATASMART_LIBS)
AM_CPPFLAGS += $(ATASMART_CFLAGS)
else
libpsensor_a_SOURCES += hdd_hddtemp.c
endif

if NVIDIA
libpsensor_a_SOURCES += nvidia.c
endif

if LIBATIADL
LIBS += $(LIBATIADL_LIBS)
AM_CPPFLAGS += $(LIBATIADL_CFLAGS)
libpsensor_a_SOURCES += amd.c
else
libpsensor_a_CFLAGS += -pedantic
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