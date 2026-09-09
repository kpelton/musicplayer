gsettingsschemadir = $(datadir)/glib-2.0/schemas
gsettingsschema_DATA = org.squidplayer.MusicPlayer.gschema.xml

all-local:
	glib-compile-schemas $(srcdir)

clean-local:
	rm -f $(srcdir)/gschemas.compiled
