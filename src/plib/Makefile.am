
AM_CPPFLAGS = \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libplib.a

libplib_a_CFLAGS = -pedantic

libplib_a_SOURCES = \
	url.c url.h \
	plib_io.h plib_io.c

if LUA
noinst_LIBRARIES += libplib_luatpl.a

libplib_luatpl_a_CFLAGS = -pedantic \
	$(LUA_CFLAGS)

libplib_luatpl_a_SOURCES = \
	plib_luatpl.h plib_luatpl.c 
endif