#include "player.h"

G_DEFINE_TYPE (GsPlayer, gs_player, G_TYPE_OBJECT)


static void gs_SecondsToReal(float fseconds, char *str);
static gint64 gs_PercentToTime(GsPlayer *me, gdouble percent);
static gboolean my_bus_callback (GstBus     *bus, GstMessage *message,gpointer    data);

static signals[5];
static void gst_new_tags                (const GstTagList *list,
				    const gchar *tag,
				    gpointer user_data);
static void gtk_widget_dispose (GObject *object);
static void gtk_widget_finalize (GObject *object);
static gboolean isPlaying(GsPlayer *me);
static gboolean gs_checkEnd(gpointer data);

typedef enum {
     TAGS,
     ERROR,
     EOS,
     NEWFILE
}SIGNALS;




#define NANO = 100000000.
static void
gtk_widget_dispose (GObject *object)
{
  
  
}
static void
gtk_widget_finalize (GObject *object)
{
     GsPlayer *player =GS_PLAYER(object);

     g_object_unref(player->play);
}

static void
gs_player_class_init (GsPlayerClass *klass)
{
     GObjectClass *object_class = G_OBJECT_CLASS (klass);
     
     signals[TAGS]= g_signal_newv ("new-tags",
				   G_TYPE_FROM_CLASS (klass),
				   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
				   NULL /* closure */,
				   NULL /* accumulator */,
				   NULL /* accumulator data */,
				   g_cclosure_marshal_VOID__VOID,
				   G_TYPE_NONE /* return_type */,
				   0     /* n_params */,
				   NULL  /* param_types */);
     
     signals[EOS]= g_signal_newv ("eof",
				  G_TYPE_FROM_CLASS (klass),
				  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
				  NULL /* closure */,
				  NULL /* accumulator */,
				  NULL /* accumulator data */,
				  g_cclosure_marshal_VOID__VOID,
				  G_TYPE_NONE /* return_type */,
				  0     /* n_params */,
				  NULL /* param_types */);

     signals[NEWFILE]= g_signal_new ("new-file",
			     G_TYPE_FROM_CLASS (klass),
			     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			     0 /* closure */,
			     NULL /* accumulator */,
			     NULL /* accumulator data */,
			     g_cclosure_marshal_VOID__STRING ,                            
			     G_TYPE_NONE /* return_tpe */,
			     1,
			     G_TYPE_STRING);


     object_class->dispose = gtk_widget_dispose;
     object_class->finalize = gtk_widget_finalize;


 } 

static void
gs_player_init (GsPlayer *me)
{
     
     me->play = gst_element_factory_make ("playbin", "playbin");
     me->isPlaying = FALSE;
     me->gotAtag = FALSE;
  
     me->bus = gst_pipeline_get_bus (GST_PIPELINE (me->play));
     gst_bus_add_watch (me->bus, my_bus_callback, me);
     
     me->taglist =  gst_tag_list_new ();
     gst_bus_set_flushing(me->bus,TRUE);
     //gs_Set_Volume(me,3);

     //add timeout to check for end of file
     g_timeout_add      (2000,
				  gs_checkEnd,
				  me);
}

GsPlayer*
gs_player_new (void)
{
  return g_object_new (GS_TYPE_PLAYER, NULL);
}


void gs_playFile(GsPlayer *me , char *location)
{

     
     gst_bus_add_watch (me->bus, my_bus_callback, me);

     gst_element_set_state (me->play, GST_STATE_NULL);
     g_object_set (G_OBJECT (me->play), "uri",location, NULL);


     g_signal_emit ((gpointer)me, signals[NEWFILE],0,(gpointer)location,G_TYPE_NONE);

     gst_element_set_state (me->play, GST_STATE_PLAYING);
     
     me->isPlaying == FALSE;
     
     me->track = NULL;
     
    
}

void gs_loadFile(GsPlayer *me , char *location)
{
     me->isPlaying == FALSE;
     gst_bus_add_watch (me->bus, my_bus_callback, me);
     me->track = NULL;
     me->gotAtag = FALSE;
     g_object_set (G_OBJECT (me->play), "uri",location, NULL);
     gst_element_set_state (me->play, GST_STATE_READY);
    
}


void gs_pause(GsPlayer *me)
{
     gst_element_set_state (me->play, GST_STATE_PAUSED);

}

void gs_pauseResume(GsPlayer *me)
{
     
     gst_element_set_state (me->play, GST_STATE_PLAYING);

}

gboolean gs_getLength(GsPlayer *me)
{
       GstFormat fmt = GST_FORMAT_TIME;
       gint64 pos, len;

       
       if(isPlaying(me)){ 
	    if (gst_element_query_position (me->play, &fmt, &pos)
		&& gst_element_query_duration (me->play, &fmt, &len)) {
		 g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
			  GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
		    }
       }
       return TRUE;
       
  
}



static gboolean gs_checkEnd(gpointer data)
{
     GsPlayer *me = (GsPlayer *)data;
     gdouble pos;


     if (isPlaying(me))
	  {	     
	       pos= gs_getPercentage(me);
	       //check to see if file is done
	       if(pos > 99.8){
	       //EOF found
	       g_signal_emit (data, signals[EOS], 0 /* details */);
	       }
	  }
     return TRUE;
}

gdouble gs_getPercentage(GsPlayer *me)
{
     GstFormat fmt = GST_FORMAT_TIME;
     gint64 pos, len;
     float p=0;
     float pos2,len2;
 
     //if(me->isPlaying == TRUE){
     if(isPlaying(me)){
	  if (gst_element_query_position (me->play, &fmt, &pos)
	      && gst_element_query_duration (me->play, &fmt, &len)) {
	       pos2=pos;
	       len2=len;
	       p= pos2/len *100;
	       return p;
	  }
     }
return 0;

}


static gboolean isPlaying(GsPlayer *me)
{
     GstState curr;
    
     gst_element_get_state(me->play,&curr,NULL,50000);
     
	  if (curr == GST_STATE_PLAYING || curr == GST_STATE_PAUSED)
	       return TRUE;
	       
	  
	  
     return FALSE;
}
static gint64 gs_PercentToTime(GsPlayer *me, gdouble percent){

     gint64 t=0;
     GstFormat fmt = GST_FORMAT_TIME;
     gint64  len;
     gchar test[200];
     gint64 sec;


     //if(me->isPlaying == TRUE){
     if(isPlaying(me)){
	       if(gst_element_query_duration (me->play, &fmt, &len)){
	  
	       //convert percent to fraction before
	       sec = (percent/100.) *len;

	       // t = (gint64)(len-sec);

	       gs_SecondsToReal(sec/1000000000.,test);
	       
	  }
     }
     return sec;


}

gboolean gs_CurrTime(GsPlayer *me, gchar *curr)
{
    gint64 t=0;
    GstFormat fmt = GST_FORMAT_TIME;
     gint64 pos, len;
     gchar real[50];
     gchar real2[50];
     

     //if(me->isPlaying == TRUE)
     if(isPlaying(me)){  
	  
	  if (gst_element_query_position (me->play, &fmt, &pos) && gst_element_query_duration (me->play, &fmt, &len))
	  {
	       gs_SecondsToReal(pos/1000000000.,real);
	       gs_SecondsToReal(len/1000000000.,real2);
	       g_sprintf(curr,"%s of %s" ,real,real2);
	  }
		return TRUE;

     }	    

       
       
     return FALSE;

}

 static void gs_SecondsToReal(float fseconds, char *str)
{

     int seconds = fseconds;
     int hours = 0;
     int minutes = 0;
     int leftover =0;

     
     
     minutes = seconds / 60.;

     leftover = (seconds - (minutes *60));
//     hours =  minutes / 60.;
     
     g_sprintf(str,"%i:%02i" ,minutes,leftover);
 


}
gboolean gs_SeakFromPercent(GsPlayer *player,gfloat percent)
{
     
     if(!gst_element_seek_simple( player->play,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,gs_PercentToTime(player,percent))) {
	  
	  return FALSE;
     }
    
     return TRUE;
 
}

void gs_Set_Volume(GsPlayer *player, gdouble value)
{
      if(g_object_class_find_property(
	      G_OBJECT_GET_CLASS(G_OBJECT(player->play)),
            "volume") != NULL)
    {

	 g_object_set(G_OBJECT(player->play),"volume",value,NULL);
    }
}

gdouble gs_Get_Volume(GsPlayer *player)
{
     gdouble volume;
     g_object_get(G_OBJECT(player->play),"volume",&volume,NULL);
     return volume;

}



static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data)
{
     

     GsPlayer *player = (GsPlayer *)data;
     GstTagList *list;
     gchar out[200];
     gchar *tag;
     GstState old, new,pend;
     gchar *str;
     GstMessage *peek;
     mtrack *track;
    

     //g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

     switch (GST_MESSAGE_TYPE (message)) {
     case GST_MESSAGE_ERROR: {
	  GError *err;
	  gchar *debug;
	  
	 
	  
	  
	  gst_message_parse_error (message, &err, &debug);
	  g_print ("Error: %s\n", err->message);
	  g_error_free (err);
	  g_free (debug);
	  
      	  break;
     }
            
     case GST_MESSAGE_TAG:
	  // player->isPlaying=TRUE;

	  gst_message_parse_tag(message,&list);
	 
	  track = (mtrack *) g_malloc(sizeof(mtrack));
	  track->title=NULL;
	  track->artist=NULL;
	  track->genre=NULL;
	  
	  if(!gst_tag_list_is_empty(list)){
	        gst_tag_list_foreach (list,gst_new_tags,track);
	  }
	 
     
	
	 
	  gst_tag_list_free (list);
	  
	  if(track->title != NULL || track->artist !=NULL || track->genre != NULL)
	  {
	       player->track = track;
	       g_signal_emit (data, signals[TAGS], 0 /* details */);
	       
	  }
	  break;

     case GST_MESSAGE_EOS:
	  player->isPlaying=FALSE;
	  
	  g_signal_emit (data, signals[EOS], 0 /* details */);
	  
	  break;
  default:

       
       
       /* unhandled message */
       break;
     }
     
     
     

    
}

mtrack * gs_get_tag(GsPlayer *player){

     if(player->track)
	  return player->track;
     else
	  return NULL;

}

static void gst_new_tags                (const GstTagList *list,
			     const gchar *tag,
			     gpointer user_data)
{
     gchar *str;
     const GValue *date;
     mtrack *track = (mtrack *)user_data;
     

     if(strcmp(tag,GST_TAG_TITLE) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_TITLE, &str) == TRUE){
	     track->title = str;
	}
     }
     else if(strcmp(tag,GST_TAG_ARTIST) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_ARTIST, &str) == TRUE){
	       track->artist = str;
	  }
     }
     else if(strcmp(tag,GST_TAG_ALBUM) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_ALBUM, &str) == TRUE){

	       
	       g_free(str);
	  }
     }
     else if(strcmp(tag,GST_TAG_GENRE) == 0){

	  if(gst_tag_list_get_string (list, GST_TAG_GENRE, &str) == TRUE){
	       track->genre = str;
	  }
     }
     else if(strcmp(tag,GST_TAG_COMMENT) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_COMMENT, &str) == TRUE){

	     
	       g_free(str);
	  }
     }
     else if(strcmp(tag,GST_TAG_DATE) == 0){
	       if((date = gst_tag_list_get_value_index(list, GST_TAG_DATE,0)) != NULL){

			 
	       }
     }
     else if(strcmp(tag,GST_TAG_AUDIO_CODEC)== 0){
	  if(gst_tag_list_get_string (list, GST_TAG_AUDIO_CODEC, &str) == TRUE){


	       g_free(str);
	       }
	  }
     else{



     }	 
	  

}

