#include "music-volume.h"
#include "player.h"


G_DEFINE_TYPE (MusicVolume, music_volume, GTK_TYPE_VOLUME_BUTTON)

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
	MusicVolume *me;
	GSettings *client;
	gdouble vol;

	me = g_object_new (MUSIC_TYPE_VOLUME, NULL);

	me->player = player;
	client = music_settings_new ();

	if((vol = g_settings_get_double (client,"volume")) >= 0){

		gtk_scale_button_set_value (GTK_SCALE_BUTTON(me),vol); 
		gs_Set_Volume(player,vol);
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
	return g_object_new (MUSIC_TYPE_VOLUME, NULL);

}

static void              value_changed                     (GtkScaleButton *button,
                                                            gdouble         value,
                                                            gpointer        user_data)
{
	GsPlayer *player = (GsPlayer *) user_data;
	GSettings *client;
	client = music_settings_new ();

	g_settings_set_double (client,"volume",value);
	gs_Set_Volume(player,value);
	g_object_unref(G_OBJECT(client));
}
