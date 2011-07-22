noinst_LIBRARIES = libpsensor_json.a

libpsensor_json_a_CFLAGS = -pedantic -Wall -Werror

libpsensor_json_a_SOURCES = \
	psensor_json.h psensor_json.c 

LIBS = ../lib/libpsensor.a \
	$(JSON_LIBS)

AM_CPPFLAGS = $(JSON_CFLAGS) -I$(top_srcdir)/src/lib 

