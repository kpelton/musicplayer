/* music-main-window.c */

#include "music-main-window.h"
#include "music-queue.h"
#include "music-song-entry.h"
#include "plugin-engine.h"
#include "tag-scanner.h"
#include "utils.h"
#include "musicplayer-icon.h"
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>


G_DEFINE_TYPE (MusicMainWindow, music_main_window, GTK_TYPE_WINDOW)

static void 
init_widgets(MusicMainWindow *self);

static void 
mwindow_expander_activate(GtkExpander *expander,
                          gpointer     user_data);

static void  
on_size_allocate (GtkWidget     *widget,
                  GtkAllocation *allocation,
                  gpointer       user_data) ;

static void 
mwindow_new_file(GsPlayer *player,
                 metadata * p_track,gpointer data);
static void
on_pause_released (GtkButton       *button,
                   gpointer         user_data);

static void
on_play_released (GtkButton       *button,
                  gpointer         user_data);
static gboolean 
key_press_cb(GtkWidget *widget,
             GdkEventKey *event,
             gpointer user_data);

static void
restore_window_size(MusicMainWindow *self);

static void 
music_main_window_get_property (GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_main_window_set_property (GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_main_window_dispose (GObject *object)
{
	/* Plugins own widgets inserted into this window. Deactivate them before
	 * GTK tears down the window hierarchy, so their cleanup sees live widgets. */
	music_plugins_engine_shutdown ();

	G_OBJECT_CLASS (music_main_window_parent_class)->dispose (object);
	gtk_main_quit();

}

static void
music_main_window_finalize (GObject *object)
{
	MusicMainWindow *self = MUSIC_MAIN_WINDOW(object);	

	g_object_unref(G_OBJECT(self->client));

	g_object_unref(G_OBJECT(self->player));
	self->player = NULL;
	//g_object_unref(G_OBJECT(self->queue));


	G_OBJECT_CLASS (music_main_window_parent_class)->finalize (object);

}

static void
music_main_window_class_init (MusicMainWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);


	object_class->get_property = music_main_window_get_property;

	object_class->set_property = music_main_window_set_property;
	object_class->dispose = music_main_window_dispose;
	object_class->finalize = music_main_window_finalize;
}

static void
music_main_window_init (MusicMainWindow *self)
{

	self->client = music_settings_new ();
	self->expanded = g_settings_get_boolean (self->client, "expanded");
	self->currsong = NULL;
	init_widgets(self);
	gtk_expander_set_expanded (GTK_EXPANDER (self->expander), self->expanded);
	if (self->expanded)
		gtk_widget_show (self->albumlabel);
	else
		gtk_widget_hide (self->albumlabel);
	restore_window_size(self);
	music_plugins_engine_init(self);
}

GtkWidget*
music_main_window_new (void)
{
	return GTK_WIDGET(g_object_new (MUSIC_TYPE_MAIN_WINDOW, NULL));
}

static void 
init_widgets(MusicMainWindow *self)
{
	GtkWidget *hbox;
	char *markup;

	//init player window
	self->player = gs_player_new();

	gtk_window_set_resizable (GTK_WINDOW(self),TRUE);

	gtk_window_set_title (GTK_WINDOW (self), ("squid player"));
	{
		GdkPixbuf *icon;

		icon = gdk_pixbuf_new_from_xpm_data(musicplayer_icon_xpm);
		if (icon != NULL)
		{
			gtk_window_set_icon(GTK_WINDOW(self), icon);
			g_object_unref(icon);
		}
	}

	//add mainvbox to mainwindow
	self->mainvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);

	gtk_container_add (GTK_CONTAINER (self), self->mainvbox);


	//song label
	self->songlabel = gtk_label_new("");

	gtk_label_set_line_wrap (GTK_LABEL(self->songlabel),TRUE);

	gtk_label_set_text(GTK_LABEL(self->songlabel),"No File Loaded");
	gtk_box_pack_start (GTK_BOX (self->mainvbox), hbox, FALSE, FALSE,0);

	//gtk_box_pack_start (GTK_BOX (hbox), test, FALSE, FALSE,0);
	gtk_box_pack_start (GTK_BOX (hbox), self->songlabel, TRUE, TRUE,5);


	//seek widget
	self->musicseek = music_seek_new_with_adj_and_player(GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 1)),self->player);



	self->player->scroll=self->musicseek;
	gtk_box_pack_start (GTK_BOX (self->mainvbox), self->musicseek, FALSE, FALSE,0);


	//queue
	self->queue = music_queue_new_with_player(self->player);

	//expander

	self->expander = gtk_expander_new("Play List");
	gtk_widget_set_hexpand (self->expander, TRUE);
	gtk_widget_set_vexpand (self->expander, TRUE);

	gtk_container_add (GTK_CONTAINER (self->expander), self->queue);
	gtk_widget_set_hexpand (self->queue, TRUE);
	gtk_widget_set_vexpand (self->queue, TRUE);



	//Hbox and buttons albumlabel

	self->mainhbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	self->pausebutton = gtk_button_new_from_icon_name ("media-playback-pause",GTK_ICON_SIZE_LARGE_TOOLBAR);

	self->playbutton  = gtk_button_new_from_icon_name ("media-playback-start",GTK_ICON_SIZE_LARGE_TOOLBAR);
	self->prevbutton = gtk_button_new_from_icon_name ("media-skip-backward",GTK_ICON_SIZE_LARGE_TOOLBAR);
	self->nextbutton  = gtk_button_new_from_icon_name ("media-skip-forward",GTK_ICON_SIZE_LARGE_TOOLBAR);
	self->volumebutton = music_volume_new_with_player(self->player);
	self->albumlabel = gtk_label_new ("");



	//packing of hbox expander in vbox

	gtk_box_pack_start (GTK_BOX (self->mainvbox), self->mainhbox, FALSE, FALSE,0);
	gtk_box_pack_start (GTK_BOX (self->mainvbox), self->expander, TRUE, TRUE,0);

	//pack into hbox
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->playbutton, FALSE, FALSE,2);
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->pausebutton, FALSE,FALSE,2);
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->prevbutton, FALSE, FALSE,2);
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->nextbutton, FALSE,FALSE,2);   
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->volumebutton,FALSE, FALSE,5);
	gtk_box_pack_start (GTK_BOX (self->mainhbox), self->albumlabel, TRUE, TRUE,10);

	//properties



	self->dwidth = 350;
	self->dhight = 250;

	gtk_window_set_resizable (GTK_WINDOW(self),TRUE);
	gtk_window_set_default_size         (GTK_WINDOW(self),
	                                     self->dwidth,
	                                     self->dhight);

	//gtk_label_set_ellipsize(GTK_LABEL(self->albumlabel),PANGO_ELLIPSIZE_END);

	//buttons
	gtk_button_set_relief(GTK_BUTTON(self->pausebutton),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(self->playbutton),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(self->prevbutton),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(self->nextbutton),GTK_RELIEF_NONE);

	//show all 

	gtk_widget_show(self->mainvbox);
	gtk_widget_show(self->mainhbox);
	gtk_widget_show(self->songlabel);
	gtk_widget_show(self->musicseek);
	gtk_widget_show(self->playbutton);
	gtk_widget_show(self->pausebutton);
	gtk_widget_show(self->volumebutton);
	gtk_widget_show(self->prevbutton);
	gtk_widget_show(self->nextbutton);
	gtk_widget_show(self->queue);
	gtk_widget_show(self->expander);
	gtk_widget_show_all(hbox);


	//signals

	g_signal_connect(self->expander, "activate",
	                 G_CALLBACK(mwindow_expander_activate),
	                 self);

	g_signal_connect(self->player, "new-file",
	                 G_CALLBACK(mwindow_new_file),
	                 self);

	g_signal_connect (self->pausebutton, "released",
	                  G_CALLBACK (on_pause_released),
	                  (gpointer)self->player);

	g_signal_connect (self->playbutton, "released",
	                  G_CALLBACK (on_play_released),
	                  (gpointer)self);

	g_signal_connect (G_OBJECT (self), "key_press_event",
	                  G_CALLBACK (key_press_cb),
	                  self);

	g_signal_connect (G_OBJECT (self->nextbutton), "released",
	                  G_CALLBACK(music_queue_next_file),
	                  (gpointer)self->queue);

	g_signal_connect (G_OBJECT (self->prevbutton), "released",
	                  G_CALLBACK(music_queue_prev_file),
	                  (gpointer)self->queue);

	g_signal_connect (self->player, "eof",
	                  G_CALLBACK(music_queue_next_file),
	                  (gpointer)self->queue);



	self->signum = g_signal_connect (self, "size-allocate",
	                                 G_CALLBACK(on_size_allocate),
	                                 (gpointer)self);



}

static gboolean 
key_press_cb (GtkWidget *widget,
              GdkEventKey *event,
              gpointer user_data)
{
	MusicMainWindow *self = (MusicMainWindow *)user_data;

	if(event->keyval == GDK_KEY_j)
	{
		make_jump_window(MUSIC_QUEUE(self->queue));
		return TRUE;
	}

	return FALSE;

}

static void
on_pause_released (GtkButton       *button,
                   gpointer         user_data)
{
	GsPlayer *player = (GsPlayer *) user_data;
	if (isPlaying(player))
		gs_pause(player);
	else
		gs_pauseResume(player);
}


static void
on_play_released (GtkButton       *button,
                  gpointer         user_data)
{
	MusicMainWindow *self = (MusicMainWindow *)user_data;
	if (isPaused (self->player))	
		gs_pauseResume(self->player);
	else
		music_queue_play_selected (MUSIC_QUEUE(self->queue));

}	



static void mwindow_expander_activate (GtkExpander *expander,
                                       gpointer     user_data)
{
	MusicMainWindow *self = (MusicMainWindow *)user_data;
	gint width;
	gint height;


	if(!gtk_expander_get_expanded(expander))
	{
		gtk_window_set_resizable (GTK_WINDOW(self),TRUE);
		height = g_settings_get_int(self->client,"window-height");
		width = g_settings_get_int(self->client,"window-width");
		g_settings_set_boolean(self->client,"expanded",TRUE);
		//if we are not running for the first time and have saved dimensions
		if( width && height)
		{
			gtk_window_resize  (GTK_WINDOW(self),
			                    width,
			                    height);
		}
		else
		{

			gtk_window_resize  (GTK_WINDOW(self),
			                    self->dwidth,
			                    500); 
		}

		gtk_widget_show(self->albumlabel);
		gtk_label_set_ellipsize(GTK_LABEL(self->albumlabel),PANGO_ELLIPSIZE_END);

		self->expanded = TRUE;

	}
	else//undo expanded
	{
		g_settings_set_boolean(self->client,"expanded",FALSE);
		gtk_widget_hide(self->albumlabel);
		self->expanded = FALSE;

	}     
}


static void mwindow_new_file (GsPlayer *player,
                              metadata* p_track,gpointer user_data)
{
	MusicMainWindow *self = (MusicMainWindow *)user_data;
	gchar title[1024];
	gchar output[1024];
	gchar *escaped=NULL;
	gchar *escaped_artist=NULL;
	gchar *escaped_title=NULL;
	GFile *file=NULL;
	gchar *markup;

	ts_metadata_free(self->currsong);
	self->currsong = ts_metadata_new();
	ts_metadata_copy(p_track,self->currsong);

	if(p_track->artist && p_track->title){
		g_snprintf(title,1023,"%s - %s",p_track->artist, p_track->title);           
		gtk_window_set_title(GTK_WINDOW(self),title);
		escaped_artist = g_markup_escape_text(p_track->artist,-1);
		escaped_title  = g_markup_escape_text(p_track->title,-1);
		markup = g_markup_printf_escaped ("<span style=\"oblique\" size=\"large\" >\n%s\n</span>", title);
		gtk_label_set_markup (GTK_LABEL (self->songlabel), markup);
		g_free (markup);

		g_free(escaped_title);
		g_free(escaped_artist);
		if(p_track->album)
		{
			g_snprintf(output,1023,"<span style=\"italic\" size=\"small\">from:%s</span>",p_track->album);
			gtk_label_set_markup(GTK_LABEL(self->albumlabel),output);
			gtk_widget_show(self->albumlabel);

		}else{
			gtk_label_set_text(GTK_LABEL(self->albumlabel),""); 

		}
	}
	else
	{
		file =g_file_new_for_uri((gchar *)p_track->uri);

		escaped   = parse_file_name(file);
		if(escaped)
		{
			gtk_label_set_text(GTK_LABEL(self->songlabel),escaped);
			markup = g_markup_printf_escaped ("<span style=\"oblique\" size=\"large\" >\n%s\n</span>", escaped);
			gtk_label_set_markup (GTK_LABEL (self->songlabel), markup);
			g_free (markup);
			gtk_label_set_text(GTK_LABEL(self->albumlabel),""); 
			gtk_window_set_title(GTK_WINDOW(self),escaped);
			g_object_unref(file);
			g_free(escaped);
		}
	}


}

void 
music_main_play_file(MusicMainWindow *self,gchar * location)
{
	gchar *valid=NULL;
	GFile *file = NULL;
	GFileInfo *info = NULL;
	GError *err=NULL;
	const gchar* filetype;

	file =g_file_new_for_commandline_arg(location);
	info = g_file_query_info(file,   G_FILE_ATTRIBUTE_STANDARD_NAME ","
	                         G_FILE_ATTRIBUTE_STANDARD_TYPE ","
	                         G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
	                         G_FILE_ATTRIBUTE_STANDARD_TARGET_URI 
	                         ,0,NULL,&err);

	if(!err)
	{
		//need to check if the file exists before adding to queue and playing
		valid =  g_file_get_uri(file);	
		//this is a hack
		filetype = g_file_info_get_attribute_string (info, 
		                                             G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
		//add_file_ext(valid,self->queue);
		if(check_type_supported(filetype)) //only play playable files
			gs_playFile(self->player,valid);
		g_free(valid);

		g_object_unref(info);
	}
	else
	{
		fprintf (stderr, "Unable to read file: %s\n", err->message);
		g_error_free (err);
	}
	g_object_unref(file);
}

static void  
on_size_allocate (GtkWidget     *widget,
                  GtkAllocation *allocation,
                  gpointer       user_data)  
{

	MusicMainWindow *self = (MusicMainWindow *)user_data;

	if(self->client && allocation->width > 0 && allocation->height > 0)
	{
		g_settings_set_int(self->client, "window-width", allocation->width);
		g_settings_set_int(self->client, "window-height", allocation->height);
	}
}

static void
restore_window_size(MusicMainWindow *self)
{
	gint width;
	gint height;

	width = g_settings_get_int(self->client, "window-width");
	height = g_settings_get_int(self->client, "window-height");

	if (width > 0 && height > 0)
		gtk_window_resize(GTK_WINDOW(self), width, height);
}
