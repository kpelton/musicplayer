/* real-test.h */

#ifndef _STATS_PLUGIN
#define _STATS_PLUGIN

#include <glib-object.h>
#include <gtk/gtk.h>
#include "music-main-window.h"
#include "../music-plugin.h"

G_BEGIN_DECLS

#define STATS_TYPE_PLUGIN stats_plugin_get_type()
#define STATS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATS_TYPE_PLUGIN, StatsPlugin)

#define STATS_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), STATS_TYPE_PLUGIN, StatsPluginClass))

#define REAL_IS_TEST(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATS_TYPE_PLUGIN))

#define STATS_IS_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), STATS_TYPE_PLUGIN))

#define STATS_PLUGIN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), STATS_TYPE_PLUGIN, StatsPluginClass))

typedef struct {
  MusicPlugin parent;
    MusicMainWindow *mw;
    gint id1;
    gint id2;
    gint id3;
    GtkWidget *hbox;
    GtkWidget *text;
    gchar *buffer;
     gint count;
    MusicQueue *queue;
} StatsPlugin;

typedef struct {
  MusicPluginClass parent_class;
} StatsPluginClass;

GType stats_plugin_get_type (void);

StatsPlugin*stats_plugin_new (void);

G_END_DECLS

#endif /* _STATS_PLUGIN */
