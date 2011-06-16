modules_flags = -avoid-version -module
pkglib_LTLIBRARIES = album_art.la
album_art_la_SOURCES = album_art.h album_art.c
album_art_la_LDFLAGS = $(modules_flags) `pkg-config --libs libsoup-2.4`
album_art_la_CPPFLAGS = $(common_defines) `pkg-config --cflags libsoup-2.4`

album_art_la_CFLAGS = $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins
