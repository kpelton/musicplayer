/* music-volume.h */

#ifndef _MUSIC_VOLUME
#define _MUSIC_VOLUME

#include <gtk/gtk.h>
#include "player.h"

G_BEGIN_DECLS

#define MUSIC_TYPE_VOLUME music_volume_get_type()

#define MUSIC_VOLUME(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_VOLUME, MusicVolume))

#define MUSIC_VOLUME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_VOLUME, MusicVolumeClass))

#define MUSIC_IS_VOLUME(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_VOLUME))

#define MUSIC_IS_VOLUME_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_VOLUME))

#define MUSIC_VOLUME_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_VOLUME, MusicVolumeClass))




typedef struct {
     GtkVolumeButton parent;
     GsPlayer *player;
} MusicVolume;

typedef struct {
  GtkVolumeButtonClass parent_class;
} MusicVolumeClass;

GType music_volume_get_type (void);

GtkWidget* music_volume_new_with_player (GsPlayer *player);
GtkWidget* music_volume_new(void);
G_END_DECLS


#endif /* _MUSIC_VOLUME */
