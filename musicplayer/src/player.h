/* gs-player.h */

#ifndef _GS_PLAYER
#define _GS_PLAYER

#include <glib-object.h>
#include <gst/gst.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GS_TYPE_PLAYER gs_player_get_type()

#define GS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GS_TYPE_PLAYER, GsPlayer))

#define GS_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GS_TYPE_PLAYER, GsPlayerClass))

#define GS_IS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GS_TYPE_PLAYER))

#define GS_IS_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GS_TYPE_PLAYER))

#define GS_PLAYER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GS_TYPE_PLAYER, GsPlayerClass))

typedef struct  {
     gchar *title;
     gchar *artist;
     gchar *genre;
     gchar *uri;
     
}mtrack;

typedef struct {
  GObject parent;

     GstElement *play;
     GstBus *bus;
     gboolean isPlaying;
     GtkWidget *scroll;
     GstTagList *taglist;
     gboolean gotAtag;
     mtrack *track;
     GstElement* fake;
     
} GsPlayer;

typedef struct {
  GObjectClass parent_class;
     guint signals[5];
} GsPlayerClass;



GType gs_player_get_type (void);

GsPlayer* gs_player_new (void);

void gs_playFile(GsPlayer *me , char *location);
void gs_pause(GsPlayer *me);
void gs_pauseResume(GsPlayer *me);
int gs_getLength(GsPlayer *me);
gdouble gs_getPercentage(GsPlayer *me);
gboolean gs_SeakFromPercent(GsPlayer *player,gfloat percent);
mtrack * gs_get_tag(GsPlayer *player);
void gs_Set_Volume(GsPlayer *player, gdouble value);
gdouble gs_Get_Volume(GsPlayer *player);
void gs_loadFile(GsPlayer *me , char *location);
G_END_DECLS

typedef enum {
     TAGS,
     ERROR,
     EOS,
}SIGNALS;


#endif /* _GS_PLAYER */
