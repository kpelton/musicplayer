#include "visual.h"

#include <math.h>
#include <gconf/gconf-client.h>

static const char PLUGIN_NAME[] = "Visualize";
static const char DESC[] = "Winamp-style spectrum visualizer";
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";

static gboolean visual_draw(GtkWidget *widget,
                            cairo_t *cr,
                            gpointer user_data);
static gboolean visual_redraw(gpointer user_data);
static void visual_player_message(GsPlayer *player,
                                  GstMessage *message,
                                  gpointer user_data);
static gboolean visual_create_pipeline(VisualPlugin *self,
                                       MusicMainWindow *mw);
static void visual_apply_layout(VisualPlugin *self);
static void visual_config_response(GtkDialog *dialog,
                                  gint response_id,
                                  gpointer user_data);
static GtkWidget *visual_plugin_get_config_window(MusicPlugin *plugin);

G_DEFINE_TYPE (VisualPlugin, visual_plugin, MUSIC_TYPE_PLUGIN);

static gdouble
clamp_level(gdouble value)
{
	if (value < 0.0)
		return 0.0;
	if (value > 1.0)
		return 1.0;
	return value;
}

static gdouble
magnitude_to_level(gdouble magnitude,
                   gint amplitude_range)
{
	/* GstSpectrum reports magnitudes in decibels. The user-controlled range
	 * determines how much headroom is shown before reaching maximum Y. */
	if (!isfinite(magnitude))
		return 0.0;
	return clamp_level((magnitude + amplitude_range) / amplitude_range);
}

static gdouble
display_level(const gdouble *levels,
              guint count,
              guint display_index,
              gint x_compression)
{
	gdouble position;
	gdouble source_range;
	guint lower;
	guint upper;
	gdouble fraction;

	if (count == 0)
		return 0.0;
	if (count == 1)
		return levels[0];

	/* Give the low end more horizontal room, as in a classic logarithmic
	 * analyzer. This keeps spoken-word and bass-heavy tracks from piling all
	 * of their visible activity against the left edge. */
	source_range = (count - 1) * x_compression / 100.0;
	position = pow((gdouble) display_index / (VISUAL_BANDS - 1), 3.0) *
	           source_range;
	lower = MIN((guint) position, count - 1);
	upper = MIN(lower + 1, count - 1);
	fraction = position - lower;
	return levels[lower] + (levels[upper] - levels[lower]) * fraction;
}

static void
visual_get_size(gint scale,
                gint *width,
                gint *height)
{
	switch (scale)
	{
		case 1:
			*width = 120;
			*height = 28;
			break;
		case 3:
			*width = 240;
			*height = 48;
			break;
		case 2:
		default:
			*width = 180;
			*height = 36;
			break;
	}
}

static void
visual_apply_layout(VisualPlugin *self)
{
	GtkWidget *parent;
	gint width;
	gint height;

	if (self->drawing_area == NULL)
		return;

	visual_get_size(self->scale, &width, &height);
	gtk_widget_set_size_request(self->drawing_area, width, height);
	parent = gtk_widget_get_parent(self->drawing_area);
	if (!GTK_IS_BOX(parent))
		return;

	/* Repack the existing widget so a preference change takes effect while
	 * the visualizer remains active. */
	g_object_ref(self->drawing_area);
	gtk_container_remove(GTK_CONTAINER(parent), self->drawing_area);
	if (self->placement_right)
		gtk_box_pack_end(GTK_BOX(parent), self->drawing_area, FALSE, FALSE, 4);
	else
	{
		gtk_box_pack_start(GTK_BOX(parent), self->drawing_area,
		                   FALSE, FALSE, 4);
		gtk_box_reorder_child(GTK_BOX(parent), self->drawing_area, 0);
	}
	gtk_widget_show(self->drawing_area);
	g_object_unref(self->drawing_area);
}

static void
visual_load_color(GConfClient *client,
                  const gchar *key,
                  GdkRGBA *color)
{
	gchar *value;

	value = gconf_client_get_string(client, key, NULL);
	if (value != NULL)
		gdk_rgba_parse(color, value);
	g_free(value);
}

static void
visual_load_preferences(VisualPlugin *self)
{
	GConfClient *client;
	gint scale;
	GConfValue *show_line_value;
	GConfValue *show_bars_value;
	GConfValue *show_outline_value;

	client = gconf_client_get_default();
	scale = gconf_client_get_int(client,
	                             "/apps/musicplayer/Visualize/scale",
	                             NULL);
	self->scale = scale >= 1 && scale <= 3 ? scale : 2;
	self->placement_right = gconf_client_get_bool(
		client, "/apps/musicplayer/Visualize/placement-right", NULL);
	show_line_value = gconf_client_get(client,
	                                   "/apps/musicplayer/Visualize/show-line",
	                                   NULL);
	if (show_line_value != NULL && show_line_value->type == GCONF_VALUE_BOOL)
		self->show_line = gconf_value_get_bool(show_line_value);
	if (show_line_value != NULL)
		gconf_value_free(show_line_value);
	show_bars_value = gconf_client_get(client,
	                                   "/apps/musicplayer/Visualize/show-bars",
	                                   NULL);
	if (show_bars_value != NULL && show_bars_value->type == GCONF_VALUE_BOOL)
		self->show_bars = gconf_value_get_bool(show_bars_value);
	if (show_bars_value != NULL)
		gconf_value_free(show_bars_value);
	show_outline_value = gconf_client_get(
		client, "/apps/musicplayer/Visualize/show-outline", NULL);
	if (show_outline_value != NULL &&
	    show_outline_value->type == GCONF_VALUE_BOOL)
		self->show_outline = gconf_value_get_bool(show_outline_value);
	if (show_outline_value != NULL)
		gconf_value_free(show_outline_value);
	self->x_compression = gconf_client_get_int(
		client, "/apps/musicplayer/Visualize/x-compression", NULL);
	if (self->x_compression < 25 || self->x_compression > 100)
		self->x_compression = 100;
	self->y_amplitude_range = gconf_client_get_int(
		client, "/apps/musicplayer/Visualize/y-amplitude-range", NULL);
	if (self->y_amplitude_range < 12 || self->y_amplitude_range > 96)
		self->y_amplitude_range = 48;
	self->sample_interval_ms = gconf_client_get_int(
		client, "/apps/musicplayer/Visualize/sample-interval-ms", NULL);
	if (self->sample_interval_ms < 10 || self->sample_interval_ms > 100)
		self->sample_interval_ms = 50;
	visual_load_color(client, "/apps/musicplayer/Visualize/line-color",
	                  &self->line_color);
	visual_load_color(client, "/apps/musicplayer/Visualize/bar-low-color",
	                  &self->bar_low_color);
	visual_load_color(client, "/apps/musicplayer/Visualize/bar-mid-color",
	                  &self->bar_mid_color);
	visual_load_color(client, "/apps/musicplayer/Visualize/bar-high-color",
	                  &self->bar_high_color);
	g_object_unref(client);
}

static void
visual_save_color(GConfClient *client,
                  const gchar *key,
                  const GdkRGBA *color)
{
	gchar *value = gdk_rgba_to_string(color);

	gconf_client_set_string(client, key, value, NULL);
	g_free(value);
}

static void
visual_save_preferences(VisualPlugin *self)
{
	GConfClient *client;

	client = gconf_client_get_default();
	gconf_client_set_int(client,
	                     "/apps/musicplayer/Visualize/scale",
	                     self->scale, NULL);
	gconf_client_set_bool(client,
	                      "/apps/musicplayer/Visualize/placement-right",
	                      self->placement_right, NULL);
	gconf_client_set_bool(client,
	                      "/apps/musicplayer/Visualize/show-line",
	                      self->show_line, NULL);
	gconf_client_set_bool(client,
	                      "/apps/musicplayer/Visualize/show-bars",
	                      self->show_bars, NULL);
	gconf_client_set_bool(client,
	                      "/apps/musicplayer/Visualize/show-outline",
	                      self->show_outline, NULL);
	gconf_client_set_int(client,
	                     "/apps/musicplayer/Visualize/x-compression",
	                     self->x_compression, NULL);
	gconf_client_set_int(client,
	                     "/apps/musicplayer/Visualize/y-amplitude-range",
	                     self->y_amplitude_range, NULL);
	gconf_client_set_int(client,
	                     "/apps/musicplayer/Visualize/sample-interval-ms",
	                     self->sample_interval_ms, NULL);
	visual_save_color(client, "/apps/musicplayer/Visualize/line-color",
	                  &self->line_color);
	visual_save_color(client, "/apps/musicplayer/Visualize/bar-low-color",
	                  &self->bar_low_color);
	visual_save_color(client, "/apps/musicplayer/Visualize/bar-mid-color",
	                  &self->bar_mid_color);
	visual_save_color(client, "/apps/musicplayer/Visualize/bar-high-color",
	                  &self->bar_high_color);
	g_object_unref(client);
}

static void
draw_bar(cairo_t *cr,
         gdouble x,
         gdouble bottom,
         gdouble width,
         gdouble top,
         const GdkRGBA *low_color,
         const GdkRGBA *mid_color,
         const GdkRGBA *high_color)
{
	gdouble segment_height = 4.0;
	gdouble y;

	for (y = bottom - segment_height; y >= top; y -= segment_height + 1.0)
	{
		gdouble ratio = (bottom - y) / (bottom - top + 1.0);

		if (ratio < 0.58)
			cairo_set_source_rgba(cr, low_color->red, low_color->green,
			                      low_color->blue, low_color->alpha);
		else if (ratio < 0.82)
			cairo_set_source_rgba(cr, mid_color->red, mid_color->green,
			                      mid_color->blue, mid_color->alpha);
		else
			cairo_set_source_rgba(cr, high_color->red, high_color->green,
			                      high_color->blue, high_color->alpha);
		cairo_rectangle(cr, x, y, width, segment_height);
		cairo_fill(cr);
	}
}

static gboolean
visual_draw(GtkWidget *widget,
            cairo_t *cr,
            gpointer user_data)
{
	VisualPlugin *self = user_data;
	GtkAllocation allocation;
	gdouble width;
	gdouble height;
	gdouble baseline;
	gdouble slot_width;
	gdouble bar_width;
	gdouble previous_x = 0.0;
	gdouble previous_y = 0.0;
	GtkWidget *title_row;
	GtkWidget *background_widget;
	guint i;

	gtk_widget_get_allocation(widget, &allocation);
	width = allocation.width;
	height = allocation.height;
	baseline = height - 8.0;

	/* Use the player's own theme background so the analyzer belongs to the
	 * title row instead of appearing as a separate black panel. */
	title_row = gtk_widget_get_parent(widget);
	background_widget = self->mw ? GTK_WIDGET(self->mw) : widget;
	gtk_render_background(gtk_widget_get_style_context(background_widget),
	                     cr, 0.0, 0.0, width, height);
	if (GTK_IS_WIDGET(title_row))
		gtk_render_background(gtk_widget_get_style_context(title_row),
		                     cr, 0.0, 0.0, width, height);
	/* Frequency runs left-to-right; amplitude runs bottom-to-top. */
	if (self->show_outline)
	{
		cairo_set_source_rgba(cr, 0.25, 0.31, 0.33, 0.55);
		cairo_set_line_width(cr, 1.0);
		cairo_rectangle(cr, 0.5, 0.5, MAX(0.0, width - 1.0),
		                MAX(0.0, height - 1.0));
		cairo_stroke(cr);
	}

	slot_width = width / VISUAL_BANDS;
	bar_width = MAX(1.0, slot_width - 2.0);
	for (i = 0; i < VISUAL_BANDS; i++)
	{
		gdouble x = i * slot_width + 1.0;
		gdouble level = display_level(self->levels, self->magnitude_count, i,
		                              self->x_compression);
		gdouble top = baseline - level * (height - 18.0);
		gdouble graph_y = baseline - level * (height - 18.0);

		if (self->show_bars)
			draw_bar(cr, x, baseline, bar_width, top,
			          &self->bar_low_color, &self->bar_mid_color,
			          &self->bar_high_color);
		if (self->show_line && i > 0)
		{
			cairo_set_source_rgba(cr, self->line_color.red,
			                      self->line_color.green,
			                      self->line_color.blue,
			                      self->line_color.alpha);
			cairo_set_line_width(cr, 1.5);
			cairo_move_to(cr, previous_x, previous_y);
			cairo_line_to(cr, x + bar_width / 2.0, graph_y);
			cairo_stroke(cr);
		}
		previous_x = x + bar_width / 2.0;
		previous_y = graph_y;
	}

	return FALSE;
}

static gboolean
visual_redraw(gpointer user_data)
{
	VisualPlugin *self = user_data;
	gboolean playing;
	guint i;

	playing = self->mw && self->mw->player &&
	          isPlaying(self->mw->player);
	for (i = 0; i < VISUAL_BANDS; i++)
	{
		if (!playing)
		{
			self->magnitudes[i] = 0.0;
			self->levels[i] *= 0.55;
			self->peaks[i] *= 0.70;
			continue;
		}
		self->levels[i] += (self->magnitudes[i] - self->levels[i]) * 0.35;
		if (self->peaks[i] < self->levels[i])
			self->peaks[i] = self->levels[i];
		else
			self->peaks[i] = MAX(0.0, self->peaks[i] - 0.025);
	}

	if (self->drawing_area)
		gtk_widget_queue_draw(self->drawing_area);
	return TRUE;
}

static void
visual_read_values(VisualPlugin *self,
                   const GValue *values)
{
	guint count;
	guint i;

	if (values == NULL)
		return;

	if (GST_VALUE_HOLDS_LIST(values))
		count = gst_value_list_get_size(values);
	else if (GST_VALUE_HOLDS_ARRAY(values))
		count = gst_value_array_get_size(values);
	else
		return;

	self->magnitude_count = MIN(count, (guint) VISUAL_BANDS);
	for (i = 0; i < self->magnitude_count; i++)
	{
		const GValue *value;

		if (GST_VALUE_HOLDS_LIST(values))
			value = gst_value_list_get_value(values, i);
		else
			value = gst_value_array_get_value(values, i);
		if (value && G_VALUE_HOLDS_DOUBLE(value))
			self->magnitudes[i] = magnitude_to_level(
				g_value_get_double(value), self->y_amplitude_range);
		else if (value && G_VALUE_HOLDS_FLOAT(value))
			self->magnitudes[i] = magnitude_to_level(
				g_value_get_float(value), self->y_amplitude_range);
	}
	for (; i < VISUAL_BANDS; i++)
		self->magnitudes[i] = 0.0;
}

static void
visual_player_message(GsPlayer *player,
                      GstMessage *message,
                      gpointer user_data)
{
	VisualPlugin *self = user_data;
	const GstStructure *structure;
	const GValue *magnitude;

	(void) player;

	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
		return;

	structure = gst_message_get_structure(message);
	if (structure == NULL ||
	    !gst_structure_has_name(structure, "spectrum"))
		return;

	magnitude = gst_structure_get_value(structure, "magnitude");
	visual_read_values(self, magnitude);
}

static gboolean
visual_create_pipeline(VisualPlugin *self,
                        MusicMainWindow *mw)
{
	GstElement *convert;
	GstElement *tee;
	GstElement *audio_queue;
	GstElement *visual_queue;
	GstElement *spectrum_sink;
	GstElement *audio_sink;
	GstPad *pad;
	GstPad *ghost;

	self->spectrum = gst_element_factory_make("spectrum", "visual-spectrum");
	convert = gst_element_factory_make("audioconvert", "visual-convert");
	tee = gst_element_factory_make("tee", "visual-tee");
	audio_queue = gst_element_factory_make("queue", "visual-audio-queue");
	visual_queue = gst_element_factory_make("queue", "visual-spectrum-queue");
	spectrum_sink = gst_element_factory_make("fakesink", "visual-spectrum-sink");
	audio_sink = gst_element_factory_make("autoaudiosink", "visual-audio-sink");
	if (self->spectrum == NULL || convert == NULL || tee == NULL ||
	    audio_queue == NULL || visual_queue == NULL || spectrum_sink == NULL ||
	    audio_sink == NULL)
	{
		g_warning("The GStreamer spectrum visualizer is unavailable");
		if (self->spectrum)
			gst_object_unref(self->spectrum);
		if (convert)
			gst_object_unref(convert);
		if (tee)
			gst_object_unref(tee);
		if (audio_queue)
			gst_object_unref(audio_queue);
		if (visual_queue)
			gst_object_unref(visual_queue);
		if (spectrum_sink)
			gst_object_unref(spectrum_sink);
		if (audio_sink)
			gst_object_unref(audio_sink);
		self->spectrum = NULL;
		return FALSE;
	}

	g_object_set(self->spectrum,
	             "bands", VISUAL_BANDS,
	             /* The preference is in milliseconds; GStreamer expects
	              * nanoseconds. Lower values make the analyzer more active. */
	             "interval", (guint64) self->sample_interval_ms * 1000000,
	             "post-messages", TRUE,
	             "message-magnitude", TRUE,
	             "message-phase", FALSE,
	             NULL);
	g_object_set(spectrum_sink, "sync", FALSE, "async", FALSE, NULL);
	self->bin = gst_bin_new("visual-bin");
	gst_bin_add_many(GST_BIN(self->bin), convert, tee, audio_queue,
	                 visual_queue, self->spectrum, spectrum_sink,
	                 audio_sink, NULL);

	/* Replace playbin's direct sink only after the analyzer bin owns its own
	 * audio sink. The player's original sink is restored on deactivation. */
	g_object_set(mw->player->play, "audio-sink", NULL, NULL);
	if (!gst_element_link(convert, tee) ||
	    !gst_element_link(tee, audio_queue) ||
	    !gst_element_link(tee, visual_queue) ||
	    !gst_element_link(audio_queue, audio_sink) ||
	    !gst_element_link_many(visual_queue, self->spectrum,
	                           spectrum_sink, NULL))
	{
		gst_object_unref(self->bin);
		self->bin = NULL;
		self->spectrum = NULL;
		g_object_set(mw->player->play, "audio-sink", mw->player->gconf, NULL);
		return FALSE;
	}

	pad = gst_element_get_static_pad(convert, "sink");
	if (pad == NULL)
	{
		g_warning("The visualizer audio branch has no sink pad");
		gst_object_unref(self->bin);
		self->bin = NULL;
		self->spectrum = NULL;
		g_object_set(mw->player->play, "audio-sink", mw->player->gconf, NULL);
		return FALSE;
	}
	ghost = gst_ghost_pad_new("sink", pad);
	gst_object_unref(pad);
	if (ghost == NULL || !gst_element_add_pad(self->bin, ghost))
	{
		if (ghost)
			gst_object_unref(ghost);
		gst_object_unref(self->bin);
		self->bin = NULL;
		self->spectrum = NULL;
		g_object_set(mw->player->play, "audio-sink", mw->player->gconf, NULL);
		return FALSE;
	}
	return TRUE;
}

MusicPluginDetails *
get_details(void)
{
	MusicPluginDetails *info = g_malloc0(sizeof(MusicPluginDetails));

	info->name = g_strdup(PLUGIN_NAME);
	info->desc = g_strdup(DESC);
	info->copyright = g_strdup(COPYRIGHT);
	info->website = g_strdup(WEBSITE);
	info->is_configurable = TRUE;
	return info;
}

static void
visual_config_response(GtkDialog *dialog,
                       gint response_id,
                       gpointer user_data)
{
	VisualPlugin *self = VISUAL_PLUGIN(user_data);
	GtkComboBox *scale_combo;
	GtkRange *compression_scale;
	GtkRange *amplitude_scale;
	GtkRange *sample_scale;
	GtkToggleButton *right_button;
	GtkToggleButton *bar_button;
	GtkToggleButton *line_button;
	GtkToggleButton *outline_button;
	GtkColorButton *line_color_button;
	GtkColorButton *bar_low_color_button;
	GtkColorButton *bar_mid_color_button;
	GtkColorButton *bar_high_color_button;
	gint selected_scale;

	if (response_id != GTK_RESPONSE_OK && response_id != GTK_RESPONSE_APPLY)
		return;

	scale_combo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT(dialog),
	                                               "visual-scale"));
	right_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog),
	                                                    "visual-right"));
	line_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog),
	                                                   "visual-line"));
	bar_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(dialog),
	                                                  "visual-bars"));
	outline_button = GTK_TOGGLE_BUTTON(g_object_get_data(
		G_OBJECT(dialog), "visual-outline"));
	compression_scale = GTK_RANGE(g_object_get_data(G_OBJECT(dialog),
	                                                 "visual-compression"));
	amplitude_scale = GTK_RANGE(g_object_get_data(G_OBJECT(dialog),
	                                               "visual-amplitude"));
	sample_scale = GTK_RANGE(g_object_get_data(G_OBJECT(dialog),
	                                           "visual-sample"));
	line_color_button = GTK_COLOR_BUTTON(g_object_get_data(
		G_OBJECT(dialog), "visual-line-color"));
	bar_low_color_button = GTK_COLOR_BUTTON(g_object_get_data(
		G_OBJECT(dialog), "visual-bar-low-color"));
	bar_mid_color_button = GTK_COLOR_BUTTON(g_object_get_data(
		G_OBJECT(dialog), "visual-bar-mid-color"));
	bar_high_color_button = GTK_COLOR_BUTTON(g_object_get_data(
		G_OBJECT(dialog), "visual-bar-high-color"));
	selected_scale = gtk_combo_box_get_active(scale_combo) + 1;
	if (selected_scale >= 1 && selected_scale <= 3)
		self->scale = selected_scale;
	self->placement_right = gtk_toggle_button_get_active(right_button);
	self->show_bars = gtk_toggle_button_get_active(bar_button);
	self->show_line = gtk_toggle_button_get_active(line_button);
	self->show_outline = gtk_toggle_button_get_active(outline_button);
	self->x_compression = CLAMP((gint)(gtk_range_get_value(compression_scale) +
	                                   0.5), 25, 100);
	self->y_amplitude_range = CLAMP(
		(gint)(gtk_range_get_value(amplitude_scale) + 0.5), 12, 96);
	self->sample_interval_ms = CLAMP(
		(gint)(gtk_range_get_value(sample_scale) + 0.5), 10, 100);
	if (self->spectrum != NULL)
		g_object_set(self->spectrum, "interval",
		             (guint64) self->sample_interval_ms * 1000000,
		             NULL);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(line_color_button),
	                           &self->line_color);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bar_low_color_button),
	                           &self->bar_low_color);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bar_mid_color_button),
	                           &self->bar_mid_color);
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bar_high_color_button),
	                           &self->bar_high_color);
	visual_save_preferences(self);
	visual_apply_layout(self);
	if (self->drawing_area)
		gtk_widget_queue_draw(self->drawing_area);
}

static GtkWidget *
visual_plugin_get_config_window(MusicPlugin *plugin)
{
	VisualPlugin *self = VISUAL_PLUGIN(plugin);
	GtkWidget *dialog;
	GtkWidget *content;
	GtkWidget *grid;
	GtkWidget *label;
	GtkWidget *scale_combo;
	GtkWidget *compression_scale;
	GtkWidget *amplitude_scale;
	GtkWidget *sample_scale;
	GtkWidget *left_button;
	GtkWidget *right_button;
	GtkWidget *bar_button;
	GtkWidget *line_button;
	GtkWidget *outline_button;
	GtkWidget *line_color_button;
	GtkWidget *bar_low_color_button;
	GtkWidget *bar_mid_color_button;
	GtkWidget *bar_high_color_button;

	dialog = gtk_dialog_new_with_buttons(
		"Visualizer Preferences",
		self->mw ? GTK_WINDOW(self->mw) : NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Apply", GTK_RESPONSE_OK,
		NULL);
	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
	gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
	gtk_box_pack_start(GTK_BOX(content), grid, TRUE, TRUE, 0);

	label = gtk_label_new("Scale:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
	scale_combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scale_combo),
	                               "Small (120 × 28)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scale_combo),
	                               "Medium (180 × 36)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scale_combo),
	                               "Large (240 × 48)");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scale_combo), self->scale - 1);
	gtk_grid_attach(GTK_GRID(grid), scale_combo, 1, 0, 1, 1);

	label = gtk_label_new("X-axis width (%):");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
	compression_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
	                                             25.0, 100.0, 5.0);
	gtk_scale_set_digits(GTK_SCALE(compression_scale), 0);
	gtk_scale_set_value_pos(GTK_SCALE(compression_scale), GTK_POS_RIGHT);
	gtk_range_set_value(GTK_RANGE(compression_scale), self->x_compression);
	gtk_grid_attach(GTK_GRID(grid), compression_scale, 1, 1, 1, 1);

	label = gtk_label_new("Y-axis range (dB):");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
	amplitude_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
	                                          12.0, 96.0, 6.0);
	gtk_scale_set_digits(GTK_SCALE(amplitude_scale), 0);
	gtk_scale_set_value_pos(GTK_SCALE(amplitude_scale), GTK_POS_RIGHT);
	gtk_range_set_value(GTK_RANGE(amplitude_scale), self->y_amplitude_range);
	gtk_grid_attach(GTK_GRID(grid), amplitude_scale, 1, 2, 1, 1);

	label = gtk_label_new("Placement:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
	left_button = gtk_radio_button_new_with_label(NULL, "Left of title");
	gtk_grid_attach(GTK_GRID(grid), left_button, 1, 3, 1, 1);
	right_button = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(left_button), "Right of title");
	gtk_grid_attach(GTK_GRID(grid), right_button, 1, 4, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_button),
	                             self->placement_right);
	bar_button = gtk_check_button_new_with_label("Show amplitude bars");
	gtk_grid_attach(GTK_GRID(grid), bar_button, 1, 5, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bar_button),
	                             self->show_bars);
	line_button = gtk_check_button_new_with_label("Show cyan amplitude line");
	gtk_grid_attach(GTK_GRID(grid), line_button, 1, 6, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(line_button),
	                             self->show_line);

	label = gtk_label_new("Line color:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 7, 1, 1);
	line_color_button = gtk_color_button_new_with_rgba(&self->line_color);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(line_color_button), FALSE);
	gtk_grid_attach(GTK_GRID(grid), line_color_button, 1, 7, 1, 1);

	label = gtk_label_new("Low bar color:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 8, 1, 1);
	bar_low_color_button = gtk_color_button_new_with_rgba(&self->bar_low_color);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(bar_low_color_button), FALSE);
	gtk_grid_attach(GTK_GRID(grid), bar_low_color_button, 1, 8, 1, 1);

	label = gtk_label_new("Mid bar color:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 9, 1, 1);
	bar_mid_color_button = gtk_color_button_new_with_rgba(&self->bar_mid_color);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(bar_mid_color_button), FALSE);
	gtk_grid_attach(GTK_GRID(grid), bar_mid_color_button, 1, 9, 1, 1);

	label = gtk_label_new("High bar color:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 10, 1, 1);
	bar_high_color_button = gtk_color_button_new_with_rgba(&self->bar_high_color);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(bar_high_color_button), FALSE);
	gtk_grid_attach(GTK_GRID(grid), bar_high_color_button, 1, 10, 1, 1);

	outline_button = gtk_check_button_new_with_label("Show grid outline");
	gtk_grid_attach(GTK_GRID(grid), outline_button, 1, 11, 1, 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(outline_button),
	                             self->show_outline);

	label = gtk_label_new("Sample interval (ms):");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 12, 1, 1);
	sample_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
	                                       10.0, 100.0, 5.0);
	gtk_scale_set_digits(GTK_SCALE(sample_scale), 0);
	gtk_scale_set_value_pos(GTK_SCALE(sample_scale), GTK_POS_RIGHT);
	gtk_range_set_value(GTK_RANGE(sample_scale), self->sample_interval_ms);
	gtk_widget_set_tooltip_text(sample_scale,
	                            "Lower values update the visualizer faster");
	gtk_grid_attach(GTK_GRID(grid), sample_scale, 1, 12, 1, 1);

	g_object_set_data(G_OBJECT(dialog), "visual-scale", scale_combo);
	g_object_set_data(G_OBJECT(dialog), "visual-right", right_button);
	g_object_set_data(G_OBJECT(dialog), "visual-bars", bar_button);
	g_object_set_data(G_OBJECT(dialog), "visual-line", line_button);
	g_object_set_data(G_OBJECT(dialog), "visual-outline", outline_button);
	g_object_set_data(G_OBJECT(dialog), "visual-compression", compression_scale);
	g_object_set_data(G_OBJECT(dialog), "visual-amplitude", amplitude_scale);
	g_object_set_data(G_OBJECT(dialog), "visual-sample", sample_scale);
	g_object_set_data(G_OBJECT(dialog), "visual-line-color", line_color_button);
	g_object_set_data(G_OBJECT(dialog), "visual-bar-low-color", bar_low_color_button);
	g_object_set_data(G_OBJECT(dialog), "visual-bar-mid-color", bar_mid_color_button);
	g_object_set_data(G_OBJECT(dialog), "visual-bar-high-color", bar_high_color_button);
	g_signal_connect(dialog, "response",
	                 G_CALLBACK(visual_config_response), self);
	gtk_widget_show_all(dialog);
	return dialog;
}

gboolean
visual_plugin_activate(MusicPlugin *plugin,
                       MusicMainWindow *mw)
{
	VisualPlugin *self = VISUAL_PLUGIN(plugin);

	self->mw = mw;
	visual_load_preferences(self);
	if (!visual_create_pipeline(self, mw))
		return FALSE;
	g_object_set(mw->player->play, "audio-sink", self->bin, NULL);

	self->player_signal_id = g_signal_connect(mw->player, "bus-message",
	                                          G_CALLBACK(visual_player_message),
	                                          self);

	self->drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(self->drawing_area, 180, 36);
	g_signal_connect(self->drawing_area, "draw",
	                 G_CALLBACK(visual_draw), self);
	/* Keep the analyzer in the compact title/display row, like the classic
	 * Winamp layout, instead of giving it a full-width playlist row. */
	if (GTK_IS_BOX(gtk_widget_get_parent(mw->songlabel)))
	{
		GtkBox *title_row = GTK_BOX(gtk_widget_get_parent(mw->songlabel));

		gtk_box_pack_start(title_row, self->drawing_area, FALSE, FALSE, 4);
		gtk_box_reorder_child(title_row, self->drawing_area, 0);
	}
	visual_apply_layout(self);
	gtk_widget_show(self->drawing_area);
	self->redraw_source = g_timeout_add(50, visual_redraw, self);
	return TRUE;
}

gboolean
visual_plugin_deactivate(MusicPlugin *plugin)
{
	VisualPlugin *self = VISUAL_PLUGIN(plugin);
	if (self->redraw_source)
	{
		g_source_remove(self->redraw_source);
		self->redraw_source = 0;
	}
	if (self->player_signal_id)
	{
		if (self->mw && self->mw->player)
			g_signal_handler_disconnect(self->mw->player, self->player_signal_id);
		self->player_signal_id = 0;
	}
	if (self->mw && self->mw->player)
	{
		g_object_set(self->mw->player->play, "audio-sink", NULL, NULL);
	}
	if (self->bin)
	{
		gst_element_set_state(self->bin, GST_STATE_NULL);
		gst_object_unref(self->bin);
		self->bin = NULL;
		self->spectrum = NULL;
	}
	if (self->mw && self->mw->player)
		g_object_set(self->mw->player->play, "audio-sink",
		             self->mw->player->gconf, NULL);
	if (self->drawing_area)
	{
		gtk_widget_destroy(self->drawing_area);
		self->drawing_area = NULL;
	}
	self->mw = NULL;
	return TRUE;
}

GType
register_music_plugin(void)
{
	return visual_plugin_get_type();
}

static void
visual_plugin_dispose(GObject *object)
{
	VisualPlugin *self = VISUAL_PLUGIN(object);

	if (self->bin || self->drawing_area)
		visual_plugin_deactivate(MUSIC_PLUGIN(self));
	G_OBJECT_CLASS(visual_plugin_parent_class)->dispose(object);
}

static void
visual_plugin_class_init(VisualPluginClass *klass)
{
	MusicPluginClass *plugin_class = MUSIC_PLUGIN_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	plugin_class->music_plugin_activate = visual_plugin_activate;
	plugin_class->music_plugin_deactivate = visual_plugin_deactivate;
	plugin_class->music_plugin_get_config_window = visual_plugin_get_config_window;
	object_class->dispose = visual_plugin_dispose;
}

static void
visual_plugin_init(VisualPlugin *self)
{
	self->magnitude_count = 0;
	self->scale = 2;
	self->placement_right = FALSE;
	self->show_bars = TRUE;
	self->show_line = TRUE;
	self->show_outline = TRUE;
	self->x_compression = 100;
	self->sample_interval_ms = 50;
	self->line_color.red = 0.45;
	self->line_color.green = 0.95;
	self->line_color.blue = 0.95;
	self->line_color.alpha = 1.0;
	self->bar_low_color.red = 0.10;
	self->bar_low_color.green = 0.85;
	self->bar_low_color.blue = 0.22;
	self->bar_low_color.alpha = 1.0;
	self->bar_mid_color.red = 0.95;
	self->bar_mid_color.green = 0.85;
	self->bar_mid_color.blue = 0.08;
	self->bar_mid_color.alpha = 1.0;
	self->bar_high_color.red = 0.95;
	self->bar_high_color.green = 0.20;
	self->bar_high_color.blue = 0.08;
	self->bar_high_color.alpha = 1.0;
}

VisualPlugin *
visual_plugin_new(void)
{
	return g_object_new(VISUAL_TYPE_PLUGIN, NULL);
}
