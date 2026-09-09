#ifndef _VISUAL_PLUGIN
#define _VISUAL_PLUGIN

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "music-main-window.h"
#include "../music-plugin.h"

G_BEGIN_DECLS

#define VISUAL_TYPE_PLUGIN visual_plugin_get_type()
#define VISUAL_PLUGIN(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), VISUAL_TYPE_PLUGIN, VisualPlugin))
#define VISUAL_PLUGIN_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), VISUAL_TYPE_PLUGIN, VisualPluginClass))
#define VISUAL_IS_PLUGIN(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), VISUAL_TYPE_PLUGIN))
#define VISUAL_IS_PLUGIN_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), VISUAL_TYPE_PLUGIN))
#define VISUAL_PLUGIN_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), VISUAL_TYPE_PLUGIN, VisualPluginClass))

#define VISUAL_BANDS 32

typedef struct {
	MusicPlugin parent;
	MusicMainWindow *mw;
	GtkWidget *drawing_area;
	GstElement *bin;
	GstElement *spectrum;
	gulong player_signal_id;
	guint redraw_source;
	gdouble magnitudes[VISUAL_BANDS];
	gdouble levels[VISUAL_BANDS];
	gdouble peaks[VISUAL_BANDS];
	guint magnitude_count;
	gint scale;
	gboolean placement_right;
	gboolean show_bars;
	gboolean show_line;
	gboolean show_outline;
	gint x_compression;
	gint y_amplitude_range;
	gint sample_interval_ms;
	GdkRGBA line_color;
	GdkRGBA bar_low_color;
	GdkRGBA bar_mid_color;
	GdkRGBA bar_high_color;
} VisualPlugin;

typedef struct {
	MusicPluginClass parent_class;
} VisualPluginClass;

GType visual_plugin_get_type(void);
GType register_music_plugin(void);

G_END_DECLS

#endif /* _VISUAL_PLUGIN */
