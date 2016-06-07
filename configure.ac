#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([psensor], [1.2.0],[jeanfi@gmail.com],[psensor],[http://wpitchoune.net/psensor])

AM_INIT_AUTOMAKE([-Wall gnu])

AC_CONFIG_SRCDIR([src/ui.c])
AC_CONFIG_HEADERS([config.h])

# m4
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

AC_SYS_LARGEFILE

# Checks lib build
AC_PROG_RANLIB
AM_PROG_AR

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([stdbool.h])

AM_GNU_GETTEXT_VERSION([0.16])
AM_GNU_GETTEXT([external])

# Gsettings
GLIB_GSETTINGS

############### common 

# Checks pthread
AC_CHECK_LIB(pthread, pthread_create)
PTHREAD_LIBS=-pthread
AC_SUBST(PTHREAD_LIBS)

# Checks sensors, required by psensor and psensor-server
AC_CHECK_LIB(sensors, sensors_init,
             [SENSORS_LIBS=-lsensors
	      AC_DEFINE([HAVE_LIBSENSORS],[1],[Use libsensors])])
AC_CHECK_HEADERS([sensors/sensors.h sensors/error.h])
AM_CONDITIONAL(SENSORS, test -n "$SENSORS_LIBS")
AC_SUBST(SENSORS_LIBS)

############### psensor 

### Required

PKG_CHECK_MODULES(X11, x11,,[AC_MSG_WARN("x11 missing psensor will not be built")])
AM_CONDITIONAL(X11, test -n "$X11_LIBS")
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(XEXT, xext,,[AC_MSG_WARN("xext missing psensor will not be built")])
AM_CONDITIONAL(XEXT, test -n "$XEXT_LIBS")
AC_SUBST(XEXT_CFLAGS)
AC_SUBST(XEXT_LIBS)

# Check GTK+ 3.0
PKG_CHECK_MODULES(GTK,
		  [gtk+-3.0 >= 3.4],,
		  [AC_MSG_WARN("gtk missing psensor will not be built")])
AM_CONDITIONAL(GTK, test -n "$GTK_LIBS")
# ensure that only allowed headers are included
GTK_CFLAGS+=" -DGTK_DISABLE_SINGLE_INCLUDES "
# ensure that no gtk/dk deprecated symbols are used
GTK_CFLAGS+=" -DGDK_DISABLE_DEPRECATED "
# do not enable flag because GtkStatusIcon is deprecated
#GTK_CFLAGS+=" -DGTK_DISABLE_DEPRECATED "
# ensure use of accessors
GTK_CFLAGS+=" -DGSEAL_ENABLE "
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Check udisks2
PKG_CHECK_MODULES(LIBUDISKS2,
		  udisks2,
		  [AC_DEFINE([HAVE_LIBUDISKS2],[1],[Use libudisks])],
		  [AC_MSG_WARN("Library udisks2 not present")])
AM_CONDITIONAL(LIBUDISKS2, test -n "$LIBUDISKS2_LIBS")
AC_SUBST(LIBUDISKS2_CFLAGS)
AC_SUBST(LIBUDISKS2_LIBS)

# Check libatasmart
PKG_CHECK_MODULES(ATASMART, libatasmart,
	          [AC_DEFINE([HAVE_ATASMART],[1],[Use libatasmart])],
               	  [AC_MSG_WARN("Library atasmart not present")])

AM_CONDITIONAL(ATASMART, test -n "$ATASMART_LIBS")
AC_SUBST(ATASMART_CFLAGS)
AC_SUBST(ATASMART_LIBS)

# Check libnotify
LIBNOTIFY_LIBS=
PKG_CHECK_MODULES(LIBNOTIFY, 
  libnotify, 
  [AC_DEFINE([HAVE_LIBNOTIFY],[1],[Use desktop notification])], 
  [AC_MSG_WARN("Desktop notification disabled")])
AM_CONDITIONAL(LIBNOTIFY, test -n "$LIBNOTIFY_LIBS")
AC_SUBST(LIBNOTIFY_CFLAGS)
AC_SUBST(LIBNOTIFY_LIBS)

# Checks AppIndicator 
APPINDICATOR_LIBS=
PKG_CHECK_MODULES(APPINDICATOR, appindicator3-0.1,
     [AC_DEFINE([HAVE_APPINDICATOR],[1],[Use AppIndicator3-0.1])],
     [AC_MSG_WARN(AppIndicator 3-0.1 not present")])
AM_CONDITIONAL(APPINDICATOR, test -n "$APPINDICATOR_LIBS")
AC_SUBST(APPINDICATOR_CFLAGS)
AC_SUBST(APPINDICATOR_LIBS)

# Check CURL, needed for remote monitoring
CURL_LIBS=
PKG_CHECK_MODULES(CURL, 
  libcurl, 
  [AC_DEFINE([HAVE_CURL],[1],[Use CURL])], 
  [AC_MSG_WARN("Remote monitoring disabled, curl missing")])
AM_CONDITIONAL(CURL, test -n "$CURL_LIBS")
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

# Check JSON, needed for remote monitoring
JSON_LIBS=
PKG_CHECK_MODULES(JSON, 
		  json-c >= 0.11.99,
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])], 
		  [AC_MSG_WARN("json-c missing")])

if test "$JSON_LIBS" == ""; then
PKG_CHECK_MODULES(JSON, 
		  json >= 0.11.99,
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])
		   AC_DEFINE([HAVE_JSON_0],[1],[Use JSON0])], 
		  [AC_MSG_WARN("json0 missing")])
fi

AM_CONDITIONAL(JSON, test -n "$JSON_LIBS")
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

# Enables remote monitoring if JSON and CURL is present
if test -n "$JSON_LIBS"; then
   if test -n "$CURL_LIBS"; then
   AC_DEFINE([HAVE_REMOTE_SUPPORT],[1],[Remote monitoring enabled])
   fi
fi

# Checks NVIDIA
# following code from sensors-applet
# sensors-applet.sourceforge.net/
AC_CHECK_HEADERS(NVCtrl/NVCtrl.h NVCtrl/NVCtrlLib.h, 
		 [ HAVE_NVIDIA=true ], [], 
                 [ 
		  #include <X11/Xlib.h>
                 ])
if test -n "${X11_LIBS}"; then
LIBS="${LIBS} -lX11 -lXext"

if test "${HAVE_NVIDIA}" = "true"; then
  AC_CHECK_LIB(XNVCtrl,XNVCTRLQueryExtension, 
                 [NVIDIA_LIBS="-lXNVCtrl -lX11 -lXext" 
                  AC_DEFINE(HAVE_NVIDIA,1,[NVidia support enabled])
		 ])			
fi
fi
AM_CONDITIONAL(NVIDIA, test -n "$NVIDIA_LIBS")
AC_SUBST(NVIDIA_CFLAGS)
AC_SUBST(NVIDIA_LIBS)

# Checks Unity
PKG_CHECK_MODULES(UNITY, 
	          unity >= 3.4.2,
		  [AC_DEFINE([HAVE_UNITY],[1],[Use Unity])], 
		  [AC_MSG_WARN(Unity not present)])
AC_SUBST(UNITY_CFLAGS)
AC_SUBST(UNITY_LIBS)
AM_CONDITIONAL(UNITY, test -n "$UNITY_LIBS")

# Check libatiadl
AC_ARG_WITH(libatiadl,
[  --with-libatiadl[=DIR]   use libatiadlxx in DIR],[
		   ATIADL_DIR=${withval}
		   CPPFLAGS="${CPPFLAGS} -I${ATIADL_DIR}/include"
	      	   LIBS="$LIBS -L/usr/lib/fglrx -L/usr/lib64/fglrx -L/usr/lib/catalyst -L/usr/lib64/catalyst"
		  ]
)

AC_CHECK_HEADERS(adl_defines.h,
	AC_CHECK_LIB(atiadlxx,ADL_Main_Control_Refresh,
		[
			AC_DEFINE(HAVE_LIBATIADL,1,[libatiadl is available])
			LIBATIADL_CFLAGS="-I${ATIADL_DIR}/include"
			LIBATIADL_LIBS="-L/usr/lib/fglrx -L/usr/lib64/fglrx -L/usr/lib/catalyst -L/usr/lib64/catalyst -latiadlxx -ldl"
		]))

AM_CONDITIONAL(LIBATIADL, test -n "$LIBATIADL_LIBS")
AC_SUBST(LIBATIADL_CFLAGS)
AC_SUBST(LIBATIADL_LIBS)

############### psensor-server

# Check libmicrohttpd, mandatory for psensor-server
LIBMICROHTTPD_LIBS=
PKG_CHECK_MODULES(LIBMICROHTTPD, 
  libmicrohttpd, 
  [AC_DEFINE([HAVE_LIBMICROHTTPD],[1],[Use libmicrohttpd])], 
  [AC_MSG_WARN("libmicrohttpd not present, psensor-server will NOT be built")])
AM_CONDITIONAL(LIBMICROHTTPD, test -n "$LIBMICROHTTPD_LIBS")
AC_SUBST(LIBMICROHTTPD_CFLAGS)
AC_SUBST(LIBMICROHTTPD_LIBS)

# Check gtop2, optional
AC_ARG_WITH(gtop,
[  --with-gtop[=yes|no]   use gtop],[
	with_gtop=$withval],[
	with_gtop="yes"
])

GTOP_LIBS=
if test "$with_gtop" = "yes"; then
PKG_CHECK_MODULES(GTOP, 
  libgtop-2.0, 
  [AC_DEFINE([HAVE_GTOP],[1],[Use GTOP])], 
  [AC_MSG_WARN("gtop not present, CPU/Memory information will NOT be available")])
fi
AM_CONDITIONAL(GTOP, test -n "$GTOP_LIBS")
AC_SUBST(GTOP_CFLAGS)
AC_SUBST(GTOP_LIBS)

AC_CONFIG_FILES([
 Makefile
 doc/Makefile
 src/Makefile
 src/glade/Makefile
 src/lib/Makefile
 src/server/Makefile
 icons/hicolor/scalable/Makefile
 icons/hicolor/14x14/Makefile
 icons/hicolor/16x16/Makefile
 icons/hicolor/22x22/Makefile
 icons/hicolor/24x24/Makefile
 icons/hicolor/32x32/Makefile
 icons/hicolor/48x48/Makefile
 icons/hicolor/64x64/Makefile
 icons/ubuntu-mono-dark/status/22/Makefile
 icons/ubuntu-mono-light/status/22/Makefile
 icons/ubuntu-mono-dark/apps/22/Makefile
 www/Makefile
 po/Makefile.in
 tests/Makefile
])

AC_CHECK_PROGS([HELP2MAN], [help2man])
AM_CONDITIONAL(HELP2MAN, test -n "$HELP2MAN")

AC_CHECK_PROG([ASCIIDOC], [asciidoc], yes)
AM_CONDITIONAL(ASCIIDOC, test -n "$ASCIIDOC")

AC_CHECK_PROG(HAVE_CPPCHECK, cppcheck, yes)
AM_CONDITIONAL(CPPCHECK, test -n "$HAVE_CPPCHECK")

AC_OUTPUT
