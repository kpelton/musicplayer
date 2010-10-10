/* music-song-entry.c */

#include "music-song-entry.h"
#include <math.h>
#include <string.h>
#include <pango/pangocairo.h>

G_DEFINE_TYPE (MusicSongEntry, music_song_entry, GTK_TYPE_DRAWING_AREA)

static gboolean
music_song_entry_expose (GtkWidget *self, GdkEventExpose *event);

static gboolean
mouse_released(GtkWidget      *widget,
               GdkEventButton *event);

#define GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), MUSIC_TYPE_SONG_ENTRY, MusicSongEntryPrivate))

typedef struct _MusicSongEntryPrivate MusicSongEntryPrivate;

struct _MusicSongEntryPrivate {
	int dummy;
	char *text;

};
enum scroll
{
	AUTO_SCROLL,
	NEVER_SCROLL
};

static void
music_song_entry_dispose (GObject *object)
{
	G_OBJECT_CLASS (music_song_entry_parent_class)->dispose (object);
}

static void
music_song_entry_finalize (GObject *object)
{
	G_OBJECT_CLASS (music_song_entry_parent_class)->finalize (object);
}

static void
music_song_entry_class_init (MusicSongEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	g_type_class_add_private (klass, sizeof (MusicSongEntryPrivate));

	object_class->dispose = music_song_entry_dispose;
	object_class->finalize = music_song_entry_finalize;


	widget_class->expose_event =music_song_entry_expose;
	widget_class->button_press_event= mouse_released;


}
static gboolean
translate_text(gpointer data)
{
	GtkWidget *widget = GTK_WIDGET(data);
	GdkRegion *region;
	widget = GTK_WIDGET (data);

	if (!widget->window) return FALSE;

	region = gdk_drawable_get_clip_region (widget->window);
	/* redraw the cairo canvas completely by exposing it */
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);

	gdk_region_destroy (region);
	return TRUE;
}
static gboolean
mouse_released(GtkWidget      *widget,
               GdkEventButton *event)
{
	MusicSongEntry *self = MUSIC_SONG_ENTRY(widget);

	if(self->type == 1) 
	{
		self->type= 0;
	}
	else
	{
		self->trans2=0;
		self->type++;
	}
	return TRUE;
}
static void
music_song_entry_init (MusicSongEntry *self)
{
	self->text = NULL;
	self->type = AUTO_SCROLL;   
	g_timeout_add(25,
	              translate_text,
	              self);
	gtk_widget_set_size_request(GTK_WIDGET(self),150,30);
	gtk_widget_set_events(GTK_WIDGET(self),GDK_BUTTON_RELEASE_MASK |GDK_BUTTON_PRESS_MASK );




}
void music_song_entry_set_text(MusicSongEntry *self,char *text)
{
	GtkWidget *widget = GTK_WIDGET(self);
	int x;
	if(self->text)	
		g_free(self->text);
	self->text = g_strdup(text);
	x=widget->allocation.width;
	self->trans2=x-10;
}

GtkWidget*
music_song_entry_new (void)
{
	return g_object_new (MUSIC_TYPE_SONG_ENTRY, NULL);
}

static void
music_song_entry_draw(GtkWidget *self,cairo_t *cr)
{
	gdouble radius,x,y;
	MusicSongEntry *  test = MUSIC_SONG_ENTRY(self);
	PangoLayout *layout;
	PangoFontDescription *desc;
	gint trans;
	gint delta =0; 

	gchar *copy=NULL;


	if(test->text)
	{
		radius = MIN (self->allocation.width / 2,
		              self->allocation.height / 2) - 5;

		x=self->allocation.width;
		y= self->allocation.height;


		layout = pango_cairo_create_layout (cr);

		if((test->type == AUTO_SCROLL && strlen(test->text) *8 >x)) 
		{

			delta =  strlen(test->text) *-8;

			test->trans2+=1;
			trans = x - test->trans2;
			if(trans <= delta)
				test->trans2=0;

			cairo_move_to(cr, trans,y/2-6); 
			pango_layout_set_text (layout, test->text, -1);

		}else{
			cairo_move_to(cr, 0,y/2-6);  

			pango_layout_set_text (layout, test->text, -1);
		}



		desc = pango_font_description_from_string ("Sans bold 10");
		pango_layout_set_font_description (layout, desc);
		pango_font_description_free (desc);
		cairo_set_source_rgb (cr, 0, 0, 0);

		pango_cairo_show_layout (cr, layout);

		/*
		 cairo_move_to(cr, 0,0); 

		 cairo_stroke(cr);

		 cairo_move_to (cr, 10, 5);

		 cairo_line_to(cr,x-10,5);
		 cairo_move_to (cr,x-10, 5);
		 cairo_curve_to (cr, x-10, 5, x, y - (y/2),x-10,y-5);


		 cairo_move_to (cr,x-10, y-5);
		 cairo_line_to(cr,10,y-5);	
		 cairo_curve_to (cr, 10, y-5, 0, y - (y/2),10,5);		


		 cairo_close_path (cr);
		 cairo_fill_preserve (cr);

		 cairo_set_line_width (cr, 2);
		 cairo_fill_preserve (cr);

		 */


		//cairo_move_to (cr, 0, y/2);
		//cairo_rel_line_to(cr,x-5,0);
		//cairo_rel_line_to(cr,0,-y/2.5);
		//cairo_rel_line_to(cr,-x,y/2.5);
		//cairo_new_sub_path (cr); cairo_arc (cr, 64, 64, 40, 0, 2*M_PI);

		//cairo_rel_line_to (cr, x, -y/2);


		// cairo_set_line_width (cr, 3.0); 
		//cairo_set_source_rgb (cr, 0, 0, 1);
		//cairo_fill_preserve (cr);
		cairo_set_source_rgb (cr, 0, 0, 0);




		if(copy)
			g_free(copy);

		g_object_unref(layout);


		cairo_stroke(cr);
	}
}
static gboolean
music_song_entry_expose (GtkWidget *self, GdkEventExpose *event)
{
	cairo_t *cr;

	/* get a cairo_t */
	cr = gdk_cairo_create (self->window);

	/* set a clip region for the expose event */
	cairo_rectangle (cr,

	                 event->area.x, event->area.y,
	                 event->area.width, event->area.height);
	cairo_clip (cr);

	music_song_entry_draw (self, cr);

	cairo_destroy (cr);

	return FALSE;

}
