/* music-main-window.h */

#ifndef _MUSIC_MAIN_WINDOW
#define _MUSIC_MAIN_WINDOW

#include <glib-object.h>
#include <gtk/gtk.h>
#include "player.h"
#include "music-queue.h"
#include "music-seek.h"
#include "music-volume.h"
G_BEGIN_DECLS

#define MUSIC_TYPE_MAIN_WINDOW music_main_window_get_type()

#define MUSIC_MAIN_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_MAIN_WINDOW, MusicMainWindow))

#define MUSIC_MAIN_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_MAIN_WINDOW, MusicMainWindowClass))

#define MUSIC_IS_MAIN_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_MAIN_WINDOW))

#define MUSIC_IS_MAIN_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_MAIN_WINDOW))

#define MUSIC_MAIN_WINDOW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_MAIN_WINDOW, MusicMainWindowClass))

typedef struct {
  GtkWindow parent;
     GsPlayer *player;
     GtkWidget *mainvbox;
     GtkWidget *mainhbox;
     GtkWidget *songlabel;
     GtkWidget *musicseek;
     GtkWidget *queue;
     GtkWidget *pausebutton;
     GtkWidget *playbutton;
     GtkWidget *volumebutton;
     gint dhight;
     gint dwidth;

} MusicMainWindow;



typedef struct {
  GtkWindowClass parent_class;
} MusicMainWindowClass;

GType music_main_window_get_type (void);

GtkWidget * music_main_window_new (void);
void music_main_play_file(MusicMainWindow *,gchar *);
G_END_DECLS

#endif /* _MUSIC_MAIN_WINDOW */
