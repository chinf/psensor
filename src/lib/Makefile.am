noinst_LIBRARIES = libpsensor.a

libpsensor_a_CFLAGS = -Wall -Werror

libpsensor_a_SOURCES = \
	amd.h \
	cpu.h \
	measure.h measure.c \
	color.h color.c \
	psensor.h psensor.c\
	hdd.h hdd.c\
	nvidia.h \
	lmsensor.h lmsensor.c

if NVIDIA
libpsensor_a_SOURCES += nvidia.c
endif

AM_CPPFLAGS=

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

EXTRA_DIST=$(libpsensor_a_SOURCES) \
	amd.c \
	cpu.c \
	nvidia.c