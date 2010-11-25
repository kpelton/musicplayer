/* music-queue.h */

#ifndef _MUSIC_QUEUE
#define _MUSIC_QUEUE

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include "player.h"
#include "tag-scanner.h"
#include "pl-reader.h"
#include "xspf-reader.h"
#include "m3u-reader.h"
#include "music-side-queue.h"




G_BEGIN_DECLS

#define MUSIC_TYPE_QUEUE music_queue_get_type()

#define MUSIC_QUEUE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_QUEUE, MusicQueue))

#define MUSIC_QUEUE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_QUEUE, MusicQueueClass))

#define MUSIC_IS_QUEUE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_QUEUE))

#define MUSIC_IS_QUEUE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_QUEUE))

#define MUSIC_QUEUE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_QUEUE, MusicQueueClass))
typedef struct _MusicQueuePrivate MusicQueuePrivate;
typedef struct {
	GtkVBoxClass parent;

    MusicQueuePrivate *priv;
	
} MusicQueue;

typedef struct {
  GtkVBoxClass parent_class;
} MusicQueueClass;

GType music_queue_get_type (void);

//public methods
GtkWidget* 
music_queue_new (void);

GtkWidget* 
music_queue_new_with_player(GsPlayer *player);

void 
add_file_ext(gchar * data,gpointer user_data);

void
make_jump_window(MusicQueue *self);

void 
music_queue_play_selected (MusicQueue *self);

gboolean 
check_type_supported(const gchar *type);

GList* 
music_queue_get_list(MusicQueue *self);

guint
music_queue_get_size(MusicQueue *self);

gint64
music_queue_get_length(MusicQueue *self);

void 
music_queue_next_file (GsPlayer *player,
           			  gpointer user_data);
void 
music_queue_prev_file(GsPlayer      *player,
                      gpointer    user_data);

//end public methods

G_END_DECLS

#endif /* _MUSIC_QUEUE */
