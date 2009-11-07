/* music-main-test.h */

#ifndef _MUSIC_SONG_ENTRY
#define _MUSIC_SONG_ENTRY

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_SONG_ENTRY music_song_entry_get_type()

#define MUSIC_SONG_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_SONG_ENTRY, MusicSongEntry))

#define MUSIC_SONG_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_SONG_ENTRY, MusicSongEntryClass))

#define MUSIC_IS_SONG_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_SONG_ENTRY))

#define MUSIC_IS_SONG_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_SONG_ENTRY))

#define MUSIC_SONG_ENTRY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_SONG_ENTRY, MusicSongEntryClass))

typedef struct {
  GtkDrawingArea parent;
  gchar * text;
  gint trans2;
	  
} MusicSongEntry;

typedef struct {
  GtkDrawingAreaClass parent_class;
} MusicSongEntryClass;

GType music_song_entry_get_type (void);

GtkWidget* music_song_entry_new (void);
void music_song_entry_set_text(MusicSongEntry *self,char *text);

G_END_DECLS
#endif
