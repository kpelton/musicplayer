modules_flags = -avoid-version -module
pkglib_LTLIBRARIES = youtube.la
youtube_la_SOURCES = youtube.h youtube.c
youtube_la_LDFLAGS = $(modules_flags)
youtube_la_CPPFLAGS = $(common_defines)

youtube_la_CFLAGS = $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS) \
	$(DEPENDENCY_CFLAGS) \
	$(AM_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/plugins
