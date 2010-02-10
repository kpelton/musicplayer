/* tag-scanner.c */

#include "tag-scanner.h"
#include "music-queue.h"
#include <string.h>
//#define DEBUG



G_DEFINE_TYPE (TagScanner, tag_scanner, G_TYPE_OBJECT)



static void ts_event_loop(TagScanner * self, GstBus *bus, metadata *data);

static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data
     );

static void gst_new_tags                (const GstTagList *list,
					 const gchar *tag,
					 gpointer user_data);

static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data);


typedef struct _TagScannerPrivate TagScannerPrivate;

struct _TagScannerPrivate {
     int dummy;
     
};

static void
tag_scanner_get_property (GObject *object, guint property_id,
			  GValue *value, GParamSpec *pspec)
{
     switch (property_id) {
     default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
     }
}

static void
tag_scanner_set_property (GObject *object, guint property_id,
			  const GValue *value, GParamSpec *pspec)
{
     switch (property_id) {
     default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
     }
}

static void
tag_scanner_dispose (GObject *object)
{
    
 
     
     G_OBJECT_CLASS (tag_scanner_parent_class)->dispose (object);
  
}

static void
tag_scanner_finalize (GObject *object)
{
    printf("Free ts\n");
     TagScanner *self = TAG_SCANNER(object);
    if(self->pipeline !=NULL)
    gst_object_unref(self->pipeline);

     
   
     G_OBJECT_CLASS (tag_scanner_parent_class)->finalize (object);
}

static void
tag_scanner_class_init (TagScannerClass *klass)
{
     GObjectClass *object_class = G_OBJECT_CLASS (klass);

  
     g_type_class_add_private (klass, sizeof (TagScannerPrivate));

     object_class->get_property = tag_scanner_get_property;
     object_class->set_property = tag_scanner_set_property;
     object_class->dispose = tag_scanner_dispose;
     object_class->finalize = tag_scanner_finalize;
}


static void ts_event_loop(TagScanner * self, GstBus *bus, metadata *data)
{
     GstMessage *message;
     gboolean val = FALSE;
     message = gst_bus_timed_pop  (bus,GST_CLOCK_TIME_NONE); 
     while( val != TRUE )
     {
	  
	  if(message != NULL)
	  {
	       val = my_bus_callback(bus,message,self);
	      
	       gst_message_unref(message);

	  }
	  if(!val)
	  message = gst_bus_timed_pop  (bus,GST_CLOCK_TIME_NONE); 
     }

    
}

//~ static gboolean isPlaying(TagScanner *self)
//~ {
     //~ GstState curr;
    
     //~ gst_element_get_state(self->pipeline,&curr,NULL,GST_SECOND/3);
     
	  //~ if (curr == GST_STATE_PLAYING)
	       //~ return TRUE;
	       
	  
	  
     //~ return FALSE;
//~ }

metadata * ts_get_metadata(gchar * uri,TagScanner * self){

     metadata *track=NULL;
     
     self->track = NULL;

    //create all
     self->pipeline = gst_pipeline_new ("pipeline");
     self->bus = gst_pipeline_get_bus (GST_PIPELINE (self->pipeline));
     self->filesrc = gst_element_factory_make ("giosrc", "source");
     self->dec  = gst_element_factory_make ("decodebin", "decodebin");
     self->fakesink = gst_element_factory_make ("fakesink", "sink");


     //signals
     g_signal_connect (self->dec, "new-decoded-pad", G_CALLBACK (cb_newpad),self->fakesink);
    

	
     //connect everything
    gst_bin_add (GST_BIN (self->pipeline), self->filesrc);
     gst_bin_add (GST_BIN (self->pipeline), self->fakesink);
     gst_bin_add (GST_BIN (self->pipeline), self->dec);

    gst_element_link (self->filesrc,self->dec);

     self->already_found = FALSE;


    g_object_set (G_OBJECT (self->filesrc), "location", uri, NULL);
     //gst_bus_add_watch (self->bus, my_bus_callback, self);
     gst_element_set_state (self->pipeline, GST_STATE_PAUSED);
     
    
     ts_event_loop(self,self->bus,track);
         
     gst_element_set_state (self->pipeline, GST_STATE_NULL);
     track = self->track;
    

    gst_object_unref(self->pipeline);
     gst_object_unref (GST_OBJECT (self->bus));	
    self->pipeline = NULL;
     return track;

}

void ts_metadata_list_free(GList *head)
{
	GList *node;

    	for(node=head; node!=NULL; node=node->next)
        		if(node->data)	
    			ts_metadata_free(node->data);

}

void ts_metadata_free(metadata *track)
{
     if(track){
	  if(track->uri)
	       g_free(track->uri);
	  if(track->title)
	       g_free(track->title);
	  if(track->artist)
	       g_free(track->artist);
	  if(track->genre)
	       g_free(track->genre);
	  if(track->album)
	  	  g_free(track->album);
	  if(track->codec)
	  	  g_free(track->codec);


     g_free(track);
     }


}
metadata * ts_metadata_new()
{
     metadata *track;
     track =(metadata *) g_malloc(sizeof(metadata));
     track->uri=NULL;
     track->title=NULL;
     track->artist=NULL;
     track->genre=NULL;
     track->album=NULL;
     track->codec=NULL;
     track->duration = 0;
     return track;    
}

static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data
     )
{
#ifdef DEBUG    
     g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
#endif
     
     TagScanner * self = (TagScanner *)data;
     GstTagList *list;
     metadata* track;


 

     switch (GST_MESSAGE_TYPE (message)) {
     case GST_MESSAGE_ERROR: {
	  GError *err;
	  gchar *debug;
	  gst_message_parse_error (message, &err, &debug);
	  g_print ("Error: %s\n", err->message);
	  g_error_free (err);
	  g_free (debug);
        return TRUE;

	  break;
     }
     case GST_MESSAGE_ASYNC_DONE:
	  return TRUE;
	  break;

     case GST_MESSAGE_TAG:
       
	  track = ts_metadata_new();

	  ts_metadata_free(self->track);
	
#ifdef DEBUG
	  printf("GOT TAG\n");
#endif
	  gst_message_parse_tag(message,&list); 
	  
	  if(!gst_tag_list_is_empty(list)){
	       gst_tag_list_foreach (list,gst_new_tags,track);
	  }
		 
	  gst_tag_list_free (list);
	  
	  if(track->title != NULL || track->artist !=NULL || track->genre != NULL )
	  {
	       self->track = track;
	       return TRUE;
	  }else{
	       ts_metadata_free(track);
	  }
	 
       

	  break;
     default:
	 return FALSE;
	  break;
      
     }

     /* we want to be notified again the next time there is a message
      * on the bus, so returning TRUE (FALSE means we want to stop watching
      * for messages on the bus and our callback should not be called again)
      */
     return FALSE;
}


static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data)
{
     GstPad *audiopad;
     GstElement *fakesink = (GstElement *) data;
 
     audiopad = gst_element_get_pad (fakesink, "sink");


     gst_pad_link (pad, audiopad);
}


static void
tag_scanner_init (TagScanner *self)
{
     printf("New ts\n");
     
}

TagScanner*
tag_scanner_new (void)
{
     return g_object_new (TAG_TYPE_SCANNER, NULL);
}



static void gst_new_tags                (const GstTagList *list,
					 const gchar *tag,
					 gpointer user_data)
{
     gchar *str;
     const GValue *date;
     metadata *track = (metadata *)user_data;
     

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
         
          else if(strcmp(tag,GST_TAG_DURATION)== 0){
	 gst_tag_list_get_uint64 (list,GST_TAG_DURATION , &(track->duration));
     }
     else{
#ifdef DEBUG
	  printf("%s\n",tag);
#endif

     }	 
	  

}
