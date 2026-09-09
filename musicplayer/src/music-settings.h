#ifndef MUSIC_SETTINGS_H
#define MUSIC_SETTINGS_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define MUSIC_SETTINGS_SCHEMA_ID "org.squidplayer.MusicPlayer"

GSettings *music_settings_new (void);

G_END_DECLS

#endif /* MUSIC_SETTINGS_H */
