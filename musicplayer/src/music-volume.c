#include "music-volume.h"
#include "player.h"
G_DEFINE_TYPE (MusicVolume, music_volume, GTK_TYPE_VOLUME_BUTTON)
//hack
static void              value_changed                     (GtkScaleButton *button,
							    gdouble         value,
							    gpointer        user_data);
static void
music_volume_class_init (MusicVolumeClass *klass)
{
}

static void
music_volume_init (MusicVolume *self)
{

 
}

GtkWidget*
music_volume_new_with_player (GsPlayer *player)
{
     GtkWidget *ret;
     MusicVolume *me;
     GConfClient* client;
     gdouble vol;
    
     me = g_object_new (MUSIC_TYPE_VOLUME, NULL);

     me->player = player;
 	client = gconf_client_get_default ();
    
     if((vol = gconf_client_get_float (client,"/apps/musicplayer/volume",NULL)) > 0){
	   gtk_scale_button_set_value (GTK_SCALE_BUTTON(me),vol); 
	}else{
	    
  	    gtk_scale_button_set_value (GTK_SCALE_BUTTON(me),gs_Get_Volume(me->player));
	}
      g_signal_connect (me, "value-changed",
                  (gpointer)value_changed,
		   player);

      g_object_unref(G_OBJECT(client));
     return GTK_WIDGET(me);
  	
}

GtkWidget*
music_volume_new(void)
{
     GtkWidget *ret;
     MusicVolume *me;
 	return g_object_new (MUSIC_TYPE_VOLUME, NULL);
    
       
}

static void              value_changed                     (GtkScaleButton *button,
                                                        gdouble         value,
                                                        gpointer        user_data)
{
     GsPlayer *player = (GsPlayer *) user_data;
     GConfClient* client;
     client = gconf_client_get_default ();
    
    gconf_client_set_float (client,"/apps/musicplayer/volume",value,NULL); 
     gs_Set_Volume(player,value);
     g_object_unref(G_OBJECT(client));
}
