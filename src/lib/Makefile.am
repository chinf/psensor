noinst_LIBRARIES = libpsensor.a

libpsensor_a_CFLAGS = -Werror

libpsensor_a_SOURCES = \
	amd.h \
	measure.h measure.c \
	color.h color.c \
	psensor.h psensor.c\
	hdd.h hdd.c\
	nvidia.h \
	lmsensor.h lmsensor.c

if NVIDIA
libpsensor_a_SOURCES += nvidia.c
endif

if LIBATIADL
LIBS += $(LIBATIADL_LIBS)
AM_CPPFLAGS = $(LIBATIADL_CFLAGS)
else
libpsensor_a_CFLAGS += -pedantic
endif

EXTRA_DIST=$(libpsensor_a_SOURCES) \
	amd.c \
	nvidia.c