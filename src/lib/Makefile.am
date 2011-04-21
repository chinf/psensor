noinst_LIBRARIES = libpsensor.a

libpsensor_a_CFLAGS = -pedantic -Werror

libpsensor_a_SOURCES = \
	measure.h measure.c \
	color.h color.c \
	psensor.h psensor.c\
	hdd.h hdd.c\
	nvidia.h \
	lmsensor.h lmsensor.c

if NVIDIA
libpsensor_a_SOURCES += nvidia.c
endif
