modules_flags = -avoid-version -module
pkglib_LTLIBRARIES = visual.la
visual_la_SOURCES = visual.h visual.c
visual_la_LDFLAGS = $(modules_flags) 
visual_la_CPPFLAGS = $(common_defines)

visual_la_CFLAGS = $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins