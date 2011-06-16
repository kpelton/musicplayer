/* real-test.h */

#ifndef _ALBUM_ART
#define _ALBUM_ART

#include <glib-object.h>
#include "music-main-window.h"
#include "../music-plugin.h"

G_BEGIN_DECLS

#define ALBUM_TYPE_ART album_art_get_type()

#define ALBUM_ART(obj) \
(G_TYPE_CHECK_INSTANCE_CAST ((obj), ALBUM_TYPE_ART, RealTest))

#define ALBUM_ART_CLASS(klass) \
(G_TYPE_CHECK_CLASS_CAST ((klass), ALBUM_TYPE_ART, AlbumArtClass))

#define ALBUM_IS_ART(obj) \
(G_TYPE_CHECK_INSTANCE_TYPE ((obj), ALBUM_TYPE_ART))

#define ALBUM_IS_ART_CLASS(klass) \
(G_TYPE_CHECK_CLASS_TYPE ((klass), ALBUM_TYPE_ART))

#define ALBUM_ART_GET_CLASS(obj) \
(G_TYPE_INSTANCE_GET_CLASS ((obj), ALBUM_TYPE_ART, AlbumArtClass))

typedef struct {
	MusicPlugin parent;
	MusicMainWindow *mw;
	gint id1;
	gint id2;
        GtkWidget *album;
        guint hash;
} AlbumArt;

typedef struct {
	MusicPluginClass parent_class;
} AlbumArtClass;

GType album_art_get_type (void);

AlbumArt* album_art_new (void);

G_END_DECLS

#endif /* _ALBUM_ART */
