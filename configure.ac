#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([psensor], [0.6.2.13],[jeanfi@gmail.com],[psensor],[http://wpitchoune.net/psensor])

AM_INIT_AUTOMAKE([-Wall -Werror gnu])

AC_CONFIG_SRCDIR([src/compat.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks lib build
AC_PROG_RANLIB

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h getopt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memmove socket strdup strtol])

AM_GNU_GETTEXT_VERSION([0.16])
AM_GNU_GETTEXT([external])

############### common 

# Checks sensors, required by psensor and psensor-server
AC_CHECK_LIB(sensors, sensors_init)
AC_CHECK_HEADERS([sensors/sensors.h sensors/errors.h])
SENSORS_LIBS=-lsensors
AC_SUBST(SENSORS_LIBS)

############### psensor 

### Required

PKG_CHECK_MODULES(X11, x11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(XEXT, xext)
AC_SUBST(XEXT_CFLAGS)
AC_SUBST(XEXT_LIBS)

# Checks GTK
GTK_LIBS=
PKG_CHECK_MODULES(GTK,
		  gtk+-3.0,
		  [AC_DEFINE([HAVE_GTK3],[1],[Use GTK3])],
		  [AC_MSG_WARN("GTK+ 3 not present")])
AM_CONDITIONAL(GTK, test -n "$GTK_LIBS")
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

if test "$GTK_LIBS" == ""; then
   PKG_CHECK_MODULES(GTK, gtk+-2.0 )
   AM_CONDITIONAL(GTK, test -n "$GTK_LIBS")
   AC_SUBST(GTK_CFLAGS)
   AC_SUBST(GTK_LIBS)
fi

# Check GCONF
PKG_CHECK_MODULES(GCONF, gconf-2.0)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

### Optional

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

if test "$APPINDICATOR_LIBS" == ""; then
   PKG_CHECK_MODULES(APPINDICATOR, 
     appindicator-0.1 <= 0.2.9 , 
     [AC_DEFINE([HAVE_APPINDICATOR],[1],[Use AppIndicator 0.2.9])
      AC_DEFINE([HAVE_APPINDICATOR_029],[1],[Use AppIndicator 0.2.9])
     ],
     [AC_MSG_WARN(AppIndicator 0.2.9 not present")])
fi

if test "$APPINDICATOR_LIBS" == ""; then
   PKG_CHECK_MODULES(APPINDICATOR, 
     appindicator-0.1 > 0.2.9, 
     [AC_DEFINE([HAVE_APPINDICATOR],[1],[Use AppIndicator > 0.2.9])], 
     [AC_MSG_WARN("AppIndicator > 0.2.9 not present")])
fi
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
  json, 
  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])], 
  [AC_MSG_WARN("Remote monitoring disabled, json missing")])
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

# libatiadl
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

# libmicrohttpd, mandatory for psensor-server
LIBMICROHTTPD_LIBS=
PKG_CHECK_MODULES(LIBMICROHTTPD, 
  libmicrohttpd, 
  [AC_DEFINE([HAVE_LIBMICROHTTPD],[1],[Use libmicrohttpd])], 
  [AC_MSG_WARN("libmicrohttpd not present, psensor-server will NOT be built")])
AM_CONDITIONAL(LIBMICROHTTPD, test -n "$LIBMICROHTTPD_LIBS")
AC_SUBST(LIBMICROHTTPD_CFLAGS)
AC_SUBST(LIBMICROHTTPD_LIBS)

# GTop, optional

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
 src/Makefile
 src/glade/Makefile
 src/lib/Makefile
 src/unity/Makefile
 src/libpsensor_json/Makefile
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

AC_OUTPUT
