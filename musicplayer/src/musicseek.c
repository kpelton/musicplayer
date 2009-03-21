/*a music-seek.c */

#include "music-seek.h"
#include <gtk/gtk.h>
G_DEFINE_TYPE (MusicSeek, music_seek, GTK_TYPE_HSCALE);
static void ChangeScroll                     ( gpointer         user_data);
static gchar * PrintTime (GtkScale *scale,
		   gdouble   arg1,
			  gpointer  user_data);
static gboolean ValueChanged (GtkRange     *range,
		       GtkScrollType scroll,
		       gdouble       value,
			      gpointer      user_data);

static void
music_seek_class_init (MusicSeekClass *klass)
{
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
  
  me= MUSIC_SEEK(curr);

  gtk_range_set_adjustment(GTK_RANGE(curr),ad);
  me->player = player;
    

  g_signal_connect ((gpointer) GTK_WIDGET(me), "change-value",
                  ValueChanged,
                 (gpointer)(me->player));

  g_signal_connect ((gpointer) GTK_WIDGET(me), "format-value",
                  PrintTime,
                 (gpointer)(me->player));

  g_timeout_add (200,ChangeScroll, me->player);
  
  return curr;
  
}

static void ChangeScroll                     ( gpointer         user_data)

{

     GsPlayer *player = (GsPlayer *) user_data;
     gdouble curr; 
     GtkAdjustment  *adj;
     
     curr= gs_getPercentage(player);
     
     adj =(GtkAdjustment  *) gtk_adjustment_new (curr, 0, 100, 3, 15, 1);
     
     gtk_range_set_adjustment(GTK_RANGE(player->scroll),GTK_ADJUSTMENT(adj));
	 
     
     
}

static gchar * PrintTime (GtkScale *scale,
		   gdouble   arg1,
		   gpointer  user_data)
     
{
     GsPlayer *player = (GsPlayer *) user_data;
     gchar *str;

     str = (gchar *) g_malloc(sizeof(gchar)*50);
     
     
     if(gs_CurrTime(player,str) == TRUE){
	  
	       	       return str;
     }else{
	  g_strlcpy(str,"0:00 of 0:00",50);
	  return str;
	  // return NULL
	       //  g_free(str);
     }


}
static gboolean ValueChanged (GtkRange     *range,
		       GtkScrollType scroll,
		       gdouble       value,
		       gpointer      user_data)
{
GsPlayer *player = (GsPlayer *) user_data;


if(!gs_SeakFromPercent(player,value) ||value > 100 || value <= 0){
     g_print ("Seek failed!\n");
     return FALSE;
     }

return FALSE;

}
