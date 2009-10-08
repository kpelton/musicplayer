#include "player.h"
#include <string.h>
 


static void gs_SecondsToReal(float fseconds, char *str);
static gint64 gs_PercentToTime(GsPlayer *me, gdouble percent);
static gboolean my_bus_callback (GstBus     *bus, GstMessage *message,gpointer    data);

static signals[5];
static void gst_new_tags                (const GstTagList *list,
				    const gchar *tag,
				    gpointer user_data);
static void gs_player_dispose (GObject *object);
static void gs_player_finalize (GObject *object);

static gboolean gs_checkEnd(gpointer data);
static void ts_event_loop(GsPlayer* self, GstBus *bus);
gboolean gs_get_tags(GsPlayer *);
static metadata * copyTrack(metadata *track);


typedef enum {
     TAGS,
     ERROR,
     EOS,
     NEWFILE
}SIGNALS;

G_DEFINE_TYPE (GsPlayer, gs_player, G_TYPE_OBJECT)
static void
gs_player_dispose (GObject *object)
{
    GsPlayer *player =GS_PLAYER(object);
	if(player->play)
    {
      gst_element_set_state (player->play, GST_STATE_NULL);
     g_object_unref(player->play);
	g_object_unref(player->gconf);
    player->play=NULL;
    
    G_OBJECT_CLASS (gs_player_parent_class)->dispose (object);
    }
}
static void
gs_player_finalize (GObject *object)
{
     GsPlayer *player =GS_PLAYER(object);

   
	G_OBJECT_CLASS (gs_player_parent_class)->finalize (object);
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
				     g_cclosure_marshal_VOID__POINTER,                            
				     G_TYPE_NONE /* return_tpe */,
				     1,
				     G_TYPE_POINTER);


     object_class->dispose = gs_player_dispose;
     object_class->finalize = gs_player_finalize;


 } 

static void
gs_player_init (GsPlayer *me)
{
     GstElement *goom;
     GstElement *xv;

    
     me->play = gst_element_factory_make ("playbin", "playbin");
	 me->gconf = gst_element_factory_make("gconfaudiosink","audio-sink");
 

   
	 g_object_set(G_OBJECT(me->play),"audio-sink",me->gconf,NULL);
    
	 	 gst_element_set_state (me->play, GST_STATE_READY);
     me->isPlaying = FALSE;
     
     me->bus = gst_pipeline_get_bus (GST_PIPELINE (me->play));
     gst_bus_add_watch (me->bus, my_bus_callback, me);
     
     g_timeout_add      (1000,
				  gs_checkEnd,
				  me);
     

     me->track = NULL;
}

GsPlayer*
gs_player_new (void)
{
     GsPlayer* out;
  out = g_object_new (GS_TYPE_PLAYER, NULL);

 
   return out;
}


void gs_playFile(GsPlayer *me , char *location)
{
     gst_element_set_state (me->play, GST_STATE_NULL);

      
    
    
     g_object_set (G_OBJECT (me->play), "uri",location, NULL);
     gst_element_set_state (me->play, GST_STATE_PLAYING);

     strcpy(me->uri,location);
         me->lock = FALSE;
	 //gst_bus_add_watch (me->bus, my_bus_callback, me);
	 ts_event_loop(me,me->bus);
	 
    
     
    
    
       
}
void freeTrack(mtrack *track)
{
     if(track){
	  if(track->uri)
	       free(track->uri);
	  if(track->title)
	       free(track->title);
	  if(track->artist)
	       free(track->artist);
	  if(track->genre)
	       free(track->genre);
	  if(track->codec)
	       free(track->codec);


     free(track);
     }
}


static metadata * copyTrack(metadata *track)
{
     metadata * newTrack = NULL;

   

     if(track){
	  newTrack = ts_metadata_new();
	 
	  if(track->uri)
	       newTrack->uri = strdup(track->uri);
	  if(track->title)
	       newTrack->title = strdup(track->title);
	  if(track->artist)
	       newTrack->artist =strdup(track->artist);
	  if(track->genre)
	       newTrack->genre =strdup (track->genre);
	  if(track->codec)
	       newTrack->codec =strdup (track->codec);
	  if(track->album)
	       newTrack->album =strdup (track->album);
	    
	  
        }

     return newTrack;
}





void gs_loadFile(GsPlayer *me , char *location)
{
     /* me->isPlaying == FALSE; */
/*      gst_bus_add_watch (me->bus, my_bus_callback, me); */
/*      me->track = NULL; */
/*      g_object_set (G_OBJECT (me->play), "uri",location, NULL); */
/*      gst_element_set_state (me->play, GST_STATE_READY) */;
    
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

       
       if(isPlaying(me) || isPaused(me)){ 
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


     if (isPlaying(me) || isPaused(me))
	  {	     
	       pos= gs_getPercentage(me);
	       //check to see if file is done
	       if(pos >= 99.8){
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
     if(isPlaying(me) || isPaused(me)){
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


gboolean isPlaying(GsPlayer *me)
{
     GstState curr;
    
     gst_element_get_state(me->play,&curr,NULL,GST_SECOND);
     
	  if (curr == GST_STATE_PLAYING)
	       return TRUE;
	       	    
     return FALSE;
}

gboolean isPaused(GsPlayer *me)
{
     GstState curr;
    
     gst_element_get_state(me->play,&curr,NULL,GST_SECOND);
     
	  if (curr == GST_STATE_PAUSED)
	       return TRUE;
	       	    
     return FALSE;
}




static gint64 gs_PercentToTime(GsPlayer *me, gdouble percent){

     gint64 t=0;
     GstFormat fmt = GST_FORMAT_TIME;
     gint64  len;
     gchar test[200];
     gint64 sec=0;

     

     //if(me->isPlaying == TRUE){
     if(isPlaying(me) || isPaused(me)  ){
	       if(gst_element_query_duration (me->play, &fmt, &len)){
	  
	       //convert percent to fraction before
	       sec = (percent/100.) *len;

	       // t = (gint64)(len-sec);

	       gs_SecondsToReal(sec/GST_SECOND,test);
	       
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
     if(isPlaying(me) || isPaused(me)){  
	 
	  if (gst_element_query_position (me->play, &fmt, &pos) && gst_element_query_duration (me->play, &fmt, &len))
	  {
		 
	       gs_SecondsToReal(pos/GST_SECOND,real);
	       gs_SecondsToReal(len/GST_SECOND,real2);
	       g_sprintf(curr,"%s of %s" ,real,real2);
		 return TRUE;
	  }
		

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

static void ts_event_loop(GsPlayer* self, GstBus *bus)
{
     GstMessage *message;
     gboolean val = FALSE;
     message = gst_bus_timed_pop(bus,GST_SECOND/3);
     metadata *track;

     track = ts_metadata_new();
     self->track = track;
     
     while( val != TRUE && message != NULL)
     {
	  
	  if(message != NULL)
	  {
	       val = my_bus_callback(bus,message,self);
	      
	       gst_message_unref(message);

	  }
	  
	  message = gst_bus_timed_pop(bus,GST_SECOND/3);
     }

     self->idle = g_idle_add    ((gpointer)gs_get_tags,
				  self);
     
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
     mtrack * track;
        
     // g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
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


	  gst_message_parse_tag(message,&list);
	 

	  if(!gst_tag_list_is_empty(list)){
	        gst_tag_list_foreach (list,gst_new_tags,player);
	  }
	  
	  gst_tag_list_free (list);

	  if(player->track->duration != 0 || player->track->codec != NULL)
	  {
	       return TRUE;
	  }
	  
	  
	  
     break;
	       
     case GST_MESSAGE_EOS:
	 	  
	  g_signal_emit (data, signals[EOS], 0 /* details */);
	  
	  break;


  default:

       return FALSE;
       
       /* unhandled message */
       break;
     }
     
     
     return FALSE;

    
}

metadata * gs_get_tag(GsPlayer *player){

     return NULL;
	

}

static void gst_new_tags                (const GstTagList *list,
			     const gchar *tag,
			     gpointer user_data)
{
     gchar *str;
     const GValue *date;
     GsPlayer *player = (GsPlayer *)user_data;
     guint dur;
     metadata *track = player->track;
 
	   


     if(strcmp(tag,GST_TAG_TITLE) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_TITLE, &str) == TRUE){
	       track->title = strdup(str);
		    g_free(str);
	}
     }
     else if(strcmp(tag,GST_TAG_ARTIST) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_ARTIST, &str) == TRUE){
	       track->artist = strdup(str);
		    g_free(str);
	  }
     }
     else if(strcmp(tag,GST_TAG_ALBUM) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_ALBUM, &str) == TRUE){

	       track->album = strdup(str);
		    g_free(str);
			       
	  }
     }
     else if(strcmp(tag,GST_TAG_GENRE) == 0){

	  if(gst_tag_list_get_string (list, GST_TAG_GENRE, &str) == TRUE){
	       track->genre =strdup( str);
		    g_free(str);
	
	  }
     }
     else if(strcmp(tag,GST_TAG_COMMENT) == 0){
	  if(gst_tag_list_get_string (list, GST_TAG_COMMENT, &str) == TRUE){

	     
	       g_free(str);
	  }
     }
     else if(strcmp(tag,GST_TAG_BITRATE) == 0){

         
     }
     else if(strcmp(tag,GST_TAG_AUDIO_CODEC)== 0){
	  if(gst_tag_list_get_string (list, GST_TAG_AUDIO_CODEC, &str) == TRUE){

	       track->codec = strdup(str);
		    g_free(str);
	  }
	  
     }
      else if(strcmp(tag,GST_TAG_DURATION)== 0){ 
	  gst_tag_list_get_uint64(list,GST_TAG_DURATION,&(track->duration)); 
	      
     
	 }else{

	  
  
     }	 
	  
    
}


gboolean gs_get_tags(GsPlayer *player)
{

     
     
	  if(!player->lock)
	  { 
	       player->track->uri = strdup(player->uri);
	       g_signal_emit (player, signals[NEWFILE],0,player->track);
	       player->lock = TRUE;
	       ts_metadata_free(player->track);
	       player->track = NULL;
	       return FALSE;
	  
	  }
	  
	  
     else{
	  return FALSE;

     }

     
}
