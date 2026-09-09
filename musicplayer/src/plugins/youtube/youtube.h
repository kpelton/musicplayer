#ifndef _YOUTUBE_PLUGIN
#define _YOUTUBE_PLUGIN

#include <gio/gio.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "music-main-window.h"
#include "../music-plugin.h"

G_BEGIN_DECLS

#define YOUTUBE_TYPE_PLUGIN youtube_plugin_get_type()
#define YOUTUBE_PLUGIN(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), YOUTUBE_TYPE_PLUGIN, YoutubePlugin))
#define YOUTUBE_PLUGIN_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), YOUTUBE_TYPE_PLUGIN, YoutubePluginClass))

typedef struct {
	MusicPlugin parent;
	MusicMainWindow *mw;
	GtkWidget *dialog;
	GtkWidget *url_entry;
	GtkWidget *status_label;
	GtkWidget *quality_combo;
	GtkWidget *download_button;
	GtkWidget *context_menu_item;
	GtkWidget *downloads_box;
	GList *downloads;
	gint audio_quality;
	guint downloads_hide_source;
} YoutubePlugin;

typedef struct {
	MusicPluginClass parent_class;
} YoutubePluginClass;

GType youtube_plugin_get_type(void);
GType register_music_plugin(void);

G_END_DECLS

#endif /* _YOUTUBE_PLUGIN */
