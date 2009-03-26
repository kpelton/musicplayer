/* music-queue.h */

#ifndef _MUSIC_QUEUE
#define _MUSIC_QUEUE

#include <glib-object.h>
#include <gtk/gtk.h>
#include "player.h"
#include "tag-scanner.h"
#include "plist-reader.h"
#include "music-queue.h"


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

typedef struct {
     GtkVBoxClass parent;
     GtkWidget* vbox;
     GtkWidget* hbox;
     GtkWidget* treeview;
     GtkWidget* openbutton;
     GtkWidget* scrolledwindow;
     GtkListStore *store;
     GsPlayer *player;
     GtkTreeIter  curr;
     GtkTreeSelection *currselection;
     GtkTreePath *path;
     gboolean changed;
     guint i;
     gint currid;
     gboolean drag_started;
     TagScanner *ts;
     PlistReader *read;
     gchar *font;
} MusicQueue;

typedef struct {
  GtkVBoxClass parent_class;
} MusicQueueClass;

GType music_queue_get_type (void);

GtkWidget* music_queue_new (void);
GtkWidget* music_queue_new_with_player(GsPlayer *player);
void add_file_ext(gpointer data,gpointer user_data);
G_END_DECLS

#endif /* _MUSIC_QUEUE */
