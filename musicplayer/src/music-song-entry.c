/* music-song-entry.c */

#include "music-song-entry.h"
#include <pango/pangocairo.h>

G_DEFINE_TYPE (MusicSongEntry, music_song_entry, GTK_TYPE_DRAWING_AREA)

enum
{
	AUTO_SCROLL,
	NEVER_SCROLL
};

static gboolean
music_song_entry_draw (GtkWidget *widget,
                       cairo_t   *cr);

static gboolean
music_song_entry_tick (gpointer user_data)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY (user_data);

	if (self->type == AUTO_SCROLL)
		self->trans2 += 2;
	gtk_widget_queue_draw (GTK_WIDGET (self));
	return G_SOURCE_CONTINUE;
}

static gboolean
music_song_entry_button_press (GtkWidget      *widget,
                               GdkEventButton *event)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY (widget);

	if (event->button != GDK_BUTTON_PRIMARY)
		return FALSE;

	if (self->type == AUTO_SCROLL)
	{
		self->type = NEVER_SCROLL;
		self->trans2 = 0;
	}
	else
	{
		self->type = AUTO_SCROLL;
	}
	gtk_widget_queue_draw (widget);
	return TRUE;
}

static void
music_song_entry_dispose (GObject *object)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY (object);

	if (self->tick_id != 0)
	{
		g_source_remove (self->tick_id);
		self->tick_id = 0;
	}

	G_OBJECT_CLASS (music_song_entry_parent_class)->dispose (object);
}

static void
music_song_entry_finalize (GObject *object)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY (object);

	g_clear_pointer (&self->text, g_free);
	G_OBJECT_CLASS (music_song_entry_parent_class)->finalize (object);
}

static void
music_song_entry_class_init (MusicSongEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = music_song_entry_dispose;
	object_class->finalize = music_song_entry_finalize;
	widget_class->draw = music_song_entry_draw;
	widget_class->button_press_event = music_song_entry_button_press;
}

static void
music_song_entry_init (MusicSongEntry *self)
{
	self->type = AUTO_SCROLL;
	self->tick_id = g_timeout_add (20, music_song_entry_tick, self);
	gtk_widget_set_size_request (GTK_WIDGET (self), 150, 30);
	gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_PRESS_MASK);
}

void
music_song_entry_set_text (MusicSongEntry *self,
                           const gchar   *text)
{
	g_return_if_fail (MUSIC_IS_SONG_ENTRY (self));

	g_free (self->text);
	self->text = g_strdup (text ? text : "");
	self->trans2 = 0;
	gtk_widget_queue_draw (GTK_WIDGET (self));
}

GtkWidget *
music_song_entry_new (void)
{
	return g_object_new (MUSIC_TYPE_SONG_ENTRY, NULL);
}

static gboolean
music_song_entry_draw (GtkWidget *widget,
                       cairo_t   *cr)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY (widget);
	PangoLayout *layout;
	PangoFontDescription *font;
	GdkRGBA color;
	gint width;
	gint height;
	gint text_width;
	gint text_height;
	gint x = 0;
	gint y;

	if (self->text == NULL || self->text[0] == '\0')
		return FALSE;

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);
	if (width <= 0 || height <= 0)
		return FALSE;

	layout = pango_cairo_create_layout (cr);
	pango_layout_set_text (layout, self->text, -1);
	font = pango_font_description_from_string ("Sans Bold 10");
	pango_layout_set_font_description (layout, font);
	pango_font_description_free (font);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);

	if (self->type == AUTO_SCROLL && text_width > width)
	{
		gint cycle = width + text_width;
		gint offset = cycle > 0 ? self->trans2 % cycle : 0;

		x = width - offset;
	}

	y = MAX (0, (height - text_height) / 2);
	cairo_save (cr);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_clip (cr);
	gtk_style_context_get_color (gtk_widget_get_style_context (widget),
	                             GTK_STATE_FLAG_NORMAL, &color);
	gdk_cairo_set_source_rgba (cr, &color);
	cairo_move_to (cr, x, y);
	pango_cairo_show_layout (cr, layout);
	cairo_restore (cr);
	g_object_unref (layout);

	return FALSE;
}
