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
     me = g_object_new (MUSIC_TYPE_VOLUME, NULL);

     me->player = player;
 
     gtk_scale_button_set_value (GTK_SCALE_BUTTON(me),gs_Get_Volume(me->player));

      g_signal_connect (me, "value-changed",
                  value_changed,
		   player);

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

     gs_Set_Volume(player,value);
     
}