
/* music-player-app.h */

#ifndef _MUSIC_PLAYER_APP
#define _MUSIC_PLAYER_APP

#include <glib-object.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_PLAYER_APP music_player_app_get_type()

#define MUSIC_PLAYER_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_PLAYER_APP, MusicPlayerApp))

#define MUSIC_PLAYER_APP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_PLAYER_APP, MusicPlayerAppClass))

#define MUSIC_IS_PLAYER_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_PLAYER_APP))

#define MUSIC_IS_PLAYER_APP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_PLAYER_APP))

#define MUSIC_PLAYER_APP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_PLAYER_APP, MusicPlayerAppClass))

typedef struct _MusicPlayerAppPrivate MusicPlayerAppPrivate;

typedef struct {
  GObject parent;
   MusicPlayerAppPrivate *priv;
} MusicPlayerApp;

typedef struct {
  GObjectClass parent_class;
} MusicPlayerAppClass;

GType music_player_app_get_type (void);

MusicPlayerApp* music_player_app_new (void);

G_END_DECLS

#endif /* _MUSIC_PLAYER_APP */