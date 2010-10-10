/* music-seek.h */

#ifndef _MUSIC_SEEK
#define _MUSIC_SEEK

#include <glib-object.h>
#include <gtk/gtk.h>
#include "player.h"
G_BEGIN_DECLS

#define MUSIC_TYPE_SEEK music_seek_get_type()

#define MUSIC_SEEK(obj) \
(G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_SEEK, MusicSeek))

#define MUSIC_SEEK_CLASS(klass) \
(G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_SEEK, MusicSeekClass))

#define MUSIC_IS_SEEK(obj) \
(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_SEEK))

#define MUSIC_IS_SEEK_CLASS(klass) \
(G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_SEEK))

#define MUSIC_SEEK_GET_CLASS(obj) \
(G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_SEEK, MusicSeekClass))

typedef struct {
	GtkHScale parent;
	GtkAdjustment *adj;
	GsPlayer *player;
} MusicSeek;

typedef struct {
	GtkHScaleClass parent_class;
} MusicSeekClass;

GType music_seek_get_type (void);

GtkWidget* music_seek_new (void);
GtkWidget *
music_seek_new_with_adj_and_player(GtkAdjustment *ad,GsPlayer *player);
G_END_DECLS

#endif /* _MUSIC_SEEK */
