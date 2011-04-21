if UNITY
noinst_LIBRARIES = libpsensor_unity.a

# no -pedantic because not supported by dee.h
libpsensor_unity_a_CFLAGS = -Wall -Werror

libpsensor_unity_a_SOURCES = \
	ui_unity.h ui_unity.c

AM_CPPFLAGS = $(UNITY_CFLAGS) \
	-I$(top_srcdir)/src/lib

LIBS += $(UNITY_LIBS)
endif