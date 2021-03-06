AC_INIT(src/main.c)
AC_CONFIG_HEADERS([config.h])
PACKAGE=musicplayer
VERSION=0.2
CFLAGS="$CFLAGS -O3"
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_HEADER_DIRENT
AM_PROG_CC_C_O

GLIB_REQS=2.0
GTK_REQS=3.0
GST_MAJORMINOR=1.0
GST_REQS=1.0
GSTPLUG_REQS=1.0

PKG_CHECK_MODULES(LIBSOUP, [libsoup-2.4 >= 2.26])
AC_SUBST(LIBSOUP_CFLAGS)
AC_SUBST(LIBSOUP_LIBS)
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0 >= 2.0 ])
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)



PKG_CHECK_MODULES(GLIB,[
  glib-2.0 >= $GLIB_REQS
  gio-2.0
  gtk+-3.0 >= $GTK_REQS
  gmodule-2.0
  gconf-2.0
  gthread-2.0
  ])

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LDFLAGS)

PKG_CHECK_MODULES(GST,[
  gstreamer-1.0 >= $GSTPLUG_REQS
  ])



AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LDFLAGS)

dnl Check for required plugins
gst010_toolsdir=`$PKG_CONFIG --variable=toolsdir gstreamer-0.10`
gst010_inspect="$gst010_toolsdir/gst-inspect-0.10"


PKG_CHECK_MODULES([GTK],[gtk+-3.0 gthread-2.0])

dnl run in source tree
AC_ARG_ENABLE(run-in-source-tree,
	      AC_HELP_STRING([--disable-run-in-source-tree],[disable search for UI files and plugins in local directory ]),
	[case "${enableval}" in
	yes) ENABLE_RUN_IN_SOURCE_TREE=yes ;;
	no)  ENABLE_RUN_IN_SOURCE_TREE=no ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --disable-run-in-source-tree) ;;
	esac],
[ENABLE_RUN_IN_SOURCE_TREE=yes]) dnl Default value
if test x$ENABLE_RUN_IN_SOURCE_TREE = xyes; then
	AC_DEFINE(TOTEM_RUN_IN_SOURCE_TREE, 1, [enable search for UI files and plugins in local directory])
fi

AC_OUTPUT([
Makefile
src/Makefile
src/plugins/Makefile
src/plugins/stats/Makefile
src/plugins/visual/Makefile
src/plugins/album/Makefile
])


