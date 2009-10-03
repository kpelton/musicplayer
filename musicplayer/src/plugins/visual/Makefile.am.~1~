modules_flags = -avoid-version -module
pkglib_LTLIBRARIES = real.la
real_la_SOURCES = real.h real.c
real_la_LDFLAGS = $(modules_flags) "-lnotify"
real_la_CPPFLAGS = $(common_defines)

real_la_CFLAGS = $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins