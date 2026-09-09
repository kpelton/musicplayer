gsettingsschemadir = $(datadir)/glib-2.0/schemas
gsettingsschema_DATA = org.squidplayer.MusicPlayer.gschema.xml

all-local:
	glib-compile-schemas $(srcdir)

install-data-hook:
	glib-compile-schemas "$(DESTDIR)$(gsettingsschemadir)"

clean-local:
	rm -f $(srcdir)/gschemas.compiled
