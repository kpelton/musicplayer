/* music-main-window.c */

#include "music-main-window.h"
#include "tag-scanner.h"
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gconf/gconf-client.h>


G_DEFINE_TYPE (MusicMainWindow, music_main_window, GTK_TYPE_WINDOW)

static void init_widgets(MusicMainWindow *self);

static void mwindow_expander_activate(GtkExpander *expander,
				      gpointer     user_data);
void            on_size_allocate                      (GtkWidget     *widget,
                                             			           GtkAllocation *allocation,
                                                      			  gpointer       user_data) ;

static void mwindow_new_file(GsPlayer *player,
					metadata * p_track,gpointer data);
static void
on_pause_released                (GtkButton       *button,
                                        gpointer         user_data);

static void
on_play_released                       (GtkButton       *button,
                                        gpointer         user_data);

static void music_main_window_get_property (GObject *object, guint property_id,
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
  G_OBJECT_CLASS (music_main_window_parent_class)->dispose (object);
 
  gtk_exit(0);
}

static void
music_main_window_finalize (GObject *object)
{

	  //unref tout
	  MusicMainWindow *self = MUSIC_MAIN_WINDOW(object);
	  g_object_unref(G_OBJECT(self->player));
	  g_object_unref(G_OBJECT(self->queue));

	  g_object_unref(G_OBJECT(self->client));		

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
  
     init_widgets(self);


		
  self->client = gconf_client_get_default ();

	  //was expanded when they quit last time 
  if(gconf_client_get_bool (self->client,"/apps/musicplayer/expanded",NULL))
	  {
            
			 mwindow_expander_activate(GTK_EXPANDER(self->expander),self);	
			 gtk_expander_set_expanded(GTK_EXPANDER(self->expander),TRUE);
			 self->expanded=TRUE;
	  }
}

GtkWidget*
music_main_window_new (void)
{
     return GTK_WIDGET(g_object_new (MUSIC_TYPE_MAIN_WINDOW, NULL));
}

static void init_widgets(MusicMainWindow *self)
{

   
     gint dwidth;
     gint dhight;

     //init player window
     self->player = gs_player_new();

     // gtk_window_set_resizable (GTK_WINDOW(self),FALSE);
		
     gtk_window_set_title (GTK_WINDOW (self), ("test"));
	
     //add mainvbox to mainwindow
     self->mainvbox = gtk_vbox_new(FALSE,0);

     gtk_container_add (GTK_CONTAINER (self), self->mainvbox);

     //song label
     self->songlabel = gtk_label_new ("No file loaded");
     gtk_box_pack_start (GTK_BOX (self->mainvbox), self->songlabel, FALSE, FALSE,0);

     //seek widget
     self->musicseek = music_seek_new_with_adj_and_player(GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 1)),self->player);


     
     self->player->scroll=self->musicseek;
     gtk_box_pack_start (GTK_BOX (self->mainvbox), self->musicseek, FALSE, FALSE,0);


     //queue
     self->queue = music_queue_new_with_player(self->player);
     
     //expander

     self->expander = gtk_expander_new("Play List");

     gtk_container_add (GTK_CONTAINER (self->expander), self->queue);
     


     //Hbox and buttons albumlabel

    self->mainhbox = gtk_hbox_new (FALSE, 0);

    self->pausebutton = gtk_button_new_from_stock ("gtk-media-pause");
    self->playbutton  = gtk_button_new_from_stock ("gtk-media-play");
    self->volumebutton = music_volume_new_with_player(self->player);
    self->albumlabel = gtk_label_new ("");

    
    //packing of hbox expander in vbox

    gtk_box_pack_start (GTK_BOX (self->mainvbox), self->mainhbox, FALSE, FALSE,0);
    gtk_box_pack_start (GTK_BOX (self->mainvbox), self->expander, TRUE, TRUE,1);

    //pack into hbox
     gtk_box_pack_start (GTK_BOX (self->mainhbox), self->pausebutton, FALSE,FALSE,0);
     gtk_box_pack_start (GTK_BOX (self->mainhbox), self->playbutton, FALSE, FALSE,0);
     gtk_box_pack_start (GTK_BOX (self->mainhbox), self->volumebutton, FALSE, FALSE,0);
     gtk_box_pack_start (GTK_BOX (self->mainhbox), self->albumlabel, TRUE, TRUE,10);

     //properties
     

     
     self->dwidth = 350;
     self->dhight = 250;

     gtk_window_set_resizable (GTK_WINDOW(self),FALSE);
     gtk_window_set_default_size         (GTK_WINDOW(self),
					  self->dwidth,
					  self->dhight);
     gtk_label_set_ellipsize(GTK_LABEL(self->songlabel),PANGO_ELLIPSIZE_END);
     gtk_label_set_ellipsize(GTK_LABEL(self->albumlabel),PANGO_ELLIPSIZE_END);
    
     //show all 

     gtk_widget_show(self->mainvbox);
     gtk_widget_show(self->mainhbox);
     gtk_widget_show(self->songlabel);
     gtk_widget_show(self->musicseek);
     gtk_widget_show(self->playbutton);
     gtk_widget_show(self->pausebutton);
     gtk_widget_show(self->volumebutton);
     gtk_widget_show(self->queue);
     gtk_widget_show(self->expander);

    
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
				  (gpointer)self->player);
	  
	  self->signum = g_signal_connect (self, "size-allocate",
				  															  G_CALLBACK(on_size_allocate),
				  															 (gpointer)self);
			 
    
}

static void
on_pause_released                (GtkButton       *button,
								gpointer         user_data)
{
    GsPlayer *player = (GsPlayer *) user_data;
    if (isPlaying(player))
    		gs_pause(player);
    else
	   gs_pauseResume(player);
}


static void
on_play_released                       (GtkButton       *button,
								gpointer         user_data)
{
    GsPlayer *player = (GsPlayer *) user_data;
    
    gs_pauseResume(player);
}



static void mwindow_expander_activate(GtkExpander *expander,
			  gpointer     user_data)
{
     MusicMainWindow *self = (MusicMainWindow *)user_data;
     gint width;
	  gint height;

	  
     if(!gtk_expander_get_expanded(expander))
     {
	  gtk_window_set_resizable (GTK_WINDOW(self),TRUE);
      height =gconf_client_get_int(self->client,"/apps/musicplayer/main_height",NULL);
	   width =gconf_client_get_int(self->client,"/apps/musicplayer/main_width",NULL);
		 gconf_client_set_bool (self->client,"/apps/musicplayer/expanded",TRUE,NULL);
			 //if we are not running for the first time and we have a key in gconf
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

	  }else//undo expande
	  {
			  gconf_client_set_bool (self->client,"/apps/musicplayer/expanded",FALSE,NULL);
			 gtk_window_set_resizable (GTK_WINDOW(self),FALSE);
			 gtk_widget_hide(self->albumlabel);
			 self->expanded = FALSE;

     }     
}


static void mwindow_new_file(GsPlayer *player,
			     metadata* p_track,gpointer user_data)
{
     MusicMainWindow *self = (MusicMainWindow *)user_data;
     gchar title[200];
     const gchar toke[] ="/";
     gchar buffer[50];
     gchar *out;
     gint i;
     gchar **tokens;
	gchar output[1024];
     gchar output2[1024];
    
    
    
    if(p_track->artist){
	   sprintf(title,"%s - %s",p_track->artist, p_track->title);
	   gtk_window_set_title(GTK_WINDOW(self),title);
	   g_sprintf(output,"<span foreground=\"blue\" size=\"large\">%s - %s</span>",p_track->artist,p_track->title);
	   gtk_label_set_markup(GTK_LABEL(self->songlabel),output);
	   if(p_track->album)
	   {
		  g_sprintf(output,"<span style=\"italic\" size=\"small\">from:%s</span>",p_track->album);
		  gtk_label_set_markup(GTK_LABEL(self->albumlabel),output);
		  gtk_widget_show(self->albumlabel);
	   }else{
		  gtk_label_set_text(GTK_LABEL(self->albumlabel),""); 
	   }
    }
    else
    {
	  g_strchomp((gchar *)p_track->uri);
	  out = gnome_vfs_get_local_path_from_uri((gchar *)p_track->uri);

	  tokens=g_strsplit(out,toke,10);

  if(tokens)
		{
	  for(i=1; tokens[i] != NULL; i++);

	  gtk_window_set_title(GTK_WINDOW(self),tokens[i-1]);
 
	  g_sprintf(output,"<span foreground=\"blue\" size=\"large\">%s</span>",tokens[i-1]);
	  gtk_label_set_markup(GTK_LABEL(self->songlabel),output);
	  g_strfreev(tokens);  
	  g_free(out);

	   gtk_label_set_text(GTK_LABEL(self->albumlabel),""); 
		}
     }
     
     ts_metadata_free(p_track);

    }

void music_main_play_file(MusicMainWindow *self,gchar * location)
{
	gchar *valid;
	valid = gnome_vfs_make_uri_from_input(location);
	add_file_ext(valid,self->queue);
	gs_playFile(self->player,valid);
	g_free(valid);
}

void            on_size_allocate                      (GtkWidget     *widget,
                                             			           GtkAllocation *allocation,
                                                      			  gpointer       user_data)  
{

		MusicMainWindow *self = (MusicMainWindow *)user_data;

	  	if(self->expanded)
	  {
	   		gconf_client_set_int                (self->client,
                                                        "/apps/musicplayer/main_width",
                                                         allocation->width,
                                                         NULL);
	    	gconf_client_set_int                (self->client,
                                                        "/apps/musicplayer/main_height",
                                                         allocation->height,
                                                         NULL);
	  }
	
}
