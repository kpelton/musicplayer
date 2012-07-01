/*a music-seek.c */

#include "music-seek.h"
#include "player.h"
#include <gtk/gtk.h>
G_DEFINE_TYPE (MusicSeek, music_seek, GTK_TYPE_HSCALE);
static void 
change_scroll ( gpointer         user_data);

static gchar * 
print_time (GtkScale *scale,
            gdouble   arg1,
            gpointer  user_data);

static gboolean 
value_changed (GtkRange     *range,
               GtkScrollType scroll,
               gdouble       value,
               gpointer      user_data);
static gboolean        
button_pressed  (GtkWidget      *widget,
                 GdkEventButton *event,
                 gpointer        user_data);
static gboolean
button_released(GtkWidget      *widget,
                GdkEventButton *event,
                gpointer        user_data);


static void
music_seek_finalize (GObject *gobject)
{
	//MusicSeek *self = MUSIC_SEEK(gobject);


	/* Chain up to the parent class */
	G_OBJECT_CLASS (music_seek_parent_class)->finalize (gobject);
}

static void
music_seek_dispose (GObject *gobject)
{
	//MusicSeek *self = MUSIC_SEEK(gobject);
	G_OBJECT_CLASS (music_seek_parent_class)->dispose (gobject);

}

static void
music_seek_class_init (MusicSeekClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->dispose = music_seek_dispose;
	gobject_class->finalize = music_seek_finalize;

}



static void
music_seek_init (MusicSeek *self)
{


	gtk_scale_set_value_pos(GTK_SCALE(self),GTK_POS_TOP);


}


GtkWidget *
music_seek_new (void)
{
	return g_object_new (MUSIC_TYPE_SEEK, NULL);
}



GtkWidget *
music_seek_new_with_adj_and_player(GtkAdjustment *ad,GsPlayer *player)
{
	GtkWidget *curr;
	MusicSeek *me;
	curr = g_object_new (MUSIC_TYPE_SEEK, NULL);
	me = MUSIC_SEEK(curr);
	me->adj = ad;
	me->player = player;
	gtk_range_set_adjustment(GTK_RANGE(me),me->adj);
	//gtk_range_set_update_policy(GTK_RANGE(me),GTK_UPDATE_DELAYED);


	g_signal_connect ((gpointer) GTK_WIDGET(me), "change-value",
	                  (gpointer)value_changed,
	                  (gpointer)(me->player));

	g_signal_connect ((gpointer) GTK_WIDGET(me), "format-value",
	                  (gpointer)print_time,
	                  (gpointer)(me->player));

	//hack to make the left click behave as middle click
	g_signal_connect ((gpointer) GTK_WIDGET(me), "button-press-event",
	                  (gpointer)button_pressed,
	                  (gpointer)(me->player));

	g_signal_connect ((gpointer) GTK_WIDGET(me), "button-release-event",
	                  (gpointer)button_released,
	                  (gpointer)(me->player));
	//end hack
	g_timeout_add (200,(gpointer)change_scroll, me->player);


	return curr;

}
static gboolean        
button_pressed  (GtkWidget      *widget,
                 GdkEventButton *event,
                 gpointer        user_data)
{
	GsPlayer *player = (GsPlayer *) user_data;
	gs_pause(player);
	event->button = 2;

	return FALSE;

}
static gboolean
button_released(GtkWidget      *widget,
                GdkEventButton *event,
                gpointer        user_data)
{
	GsPlayer *player = (GsPlayer *) user_data;
	gs_pauseResume(player);
	event->button = 2;
	return FALSE;

}


static void 
change_scroll( gpointer  user_data)

{

	GsPlayer *player = (GsPlayer *) user_data;
	gdouble curr; 
	GtkAdjustment  *adj;

	curr= gs_getPercentage(player);

	adj =(GtkAdjustment  *) gtk_adjustment_new (curr, 0, 100, 3, 10, 1);

	gtk_range_set_adjustment(GTK_RANGE(player->scroll),GTK_ADJUSTMENT(adj));



}

static gchar * 
print_time (GtkScale *scale,
            gdouble   arg1,
            gpointer  user_data)

{
	GsPlayer *player = (GsPlayer *) user_data;
	gchar *str;

	str = (gchar *) g_malloc(sizeof(gchar)*50);


	if(gs_CurrTime(player,str) == TRUE){

		return str;
	}
	else
	{
		g_strlcpy(str,"0:00 of 0:00",50);
		return str;
		gs_pauseResume(player);
	}


}
static gboolean 
value_changed (GtkRange     *range,
               GtkScrollType scroll,
               gdouble       value,
               gpointer      user_data)
{
	GsPlayer *player = (GsPlayer *) user_data;


	if(value > 100 ||value <0 ||!gs_SeakFromPercent(player,value)) {
		g_print ("Seek failed!\n");
		return FALSE;
	}

	return FALSE;

}
