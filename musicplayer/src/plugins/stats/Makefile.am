modules_flags = -avoid-version -module
pkglib_LTLIBRARIES = stats.la
stats_la_SOURCES = stats.h stats.c
stats_la_LDFLAGS = $(modules_flags) "-lnotify"
stats_la_CPPFLAGS = $(common_defines)

stats_la_CFLAGS = $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins


