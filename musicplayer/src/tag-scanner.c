/* tag-scanner.c */

#include "tag-scanner.h"
#include "music-queue.h"
#include <string.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>

//#define DEBUG



G_DEFINE_TYPE (TagScanner, tag_scanner, G_TYPE_OBJECT)

//#define GET_PRIVATE(o)					/		
//    (G_TYPE_INSTANCE_GET_PRIVATE ((o), TAG_TYPE_SCANNER, TagScannerPrivate))
//static gboolean isPlaying(TagScanner *self);
static void ts_event_loop(TagScanner * self, GstBus *bus, metadata *data);

static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data
     );

static void gst_new_tags                (const GstTagList *list,
					 const gchar *tag,
					 gpointer user_data);


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
     TagScanner *self = TAG_SCANNER(object);
     gst_element_set_state (self->pipeline, GST_STATE_NULL);
     //unref pipeline to free memory
     g_object_unref(self->pipeline);
     
     G_OBJECT_CLASS (tag_scanner_parent_class)->dispose (object);
  
}

static void
tag_scanner_finalize (GObject *object)
{
     TagScanner *self = TAG_SCANNER(object);
   
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
     message = gst_bus_timed_pop(bus,GST_SECOND/3);
     
     while( val != TRUE && message != NULL)
     {
	  
	  if(message != NULL)
	  {
	       val = my_bus_callback(bus,message,self);
	      
	       gst_message_unref(message);

	  }
	  
	  message = gst_bus_timed_pop(bus,GST_SECOND/3);
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

     metadata *track;
     
     self->track = NULL;
     
     
     
     self->bus = gst_pipeline_get_bus (GST_PIPELINE (self->pipeline)); 
     
     
     gst_element_set_state (self->pipeline, GST_STATE_READY);
     g_object_set (G_OBJECT (self->filesrc), "location", uri, NULL);
     gst_bus_add_watch (self->bus, my_bus_callback, self);
     gst_element_set_state (self->pipeline, GST_STATE_PLAYING);
     
     usleep(20000);
     ts_event_loop(self,self->bus,track);
         
     gst_element_set_state (self->pipeline, GST_STATE_NULL);
     track = self->track;

     //if(track->artist == NULL || track->title == NULL)
     //  ts_parse_file_name(track,uri);


     return track;

}
metadata * ts_parse_file_name( gchar *uri)
{
     
     gchar *parse;
     gchar *parse2;
     gchar *parse3;
     gchar *artist;
     gchar *out;
     gchar **tokens;
     char *saveptr1,*saveptr2;
     gint i = 0;
     gint j = 0;
     metadata *track = NULL;
     
     const gchar toke[] ="/";
     const gchar toke2[] ="-";
     tokens = g_malloc(sizeof(char) * 20);
     if( uri)
     {
     out = gnome_vfs_get_local_path_from_uri(uri);
    
     parse = strtok_r(out,"/",&saveptr1);
     
     while((parse = strtok_r(NULL,"/",&saveptr1)) != NULL){
	  tokens[i] = parse;
	  i++;
     }
     parse2 = strtok_r(tokens[i-1],"-",&saveptr1);


     if(parse2 != NULL)
     {
	  track = ts_metadata_new();
	  parse2 = strtok_r(NULL,"-",&saveptr1);
	  parse3 = strtok_r(parse2,".",&saveptr2);

	  printf("Artist:%s\tTitle:%s\n",tokens[i-1], parse3);
	  track->title = g_malloc(strlen(tokens[i-1]+1));
	  track->artist = g_malloc(strlen(parse2+1));			  
	  strcpy(track->artist,tokens[i-1]);
	  strcpy(track->title,parse2);
     }
     g_free(tokens);
     g_free(out);
     
     }
     return track;
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
     //g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

     
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

	  break;
     }
     case GST_MESSAGE_ASYNC_DONE:
	  //return TRUE;
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
	  
	  if(track->title != NULL || track->artist !=NULL || track->genre != NULL)
	  {
	       self->track = track;
	       return TRUE;
	  }else{
	       ts_metadata_free(track);
	  }
	 
       

	  break;
     default:
	 
	  break;
      
     }

     /* we want to be notified again the next time there is a message
      * on the bus, so returning TRUE (FALSE means we want to stop watching
      * for messages on the bus and our callback should not be called again)
      */
     return FALSE;
}
static void 
cb_gst_eos (GstElement *element, TagScanner *self)
{

     gst_element_set_state (self->pipeline, GST_STATE_NULL);
}


static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data)
{
     GstCaps *caps;
     GstStructure *str;
     GstPad *audiopad;
     GstElement *fakesink = (GstElement *) data;

 
     audiopad = gst_element_get_static_pad (fakesink , "sink");
     if (GST_PAD_IS_LINKED (audiopad)) {
	  g_object_unref (audiopad);
	  return;
     }

  
     /* link'n'play */
     gst_pad_link (pad, audiopad);
}


static void
tag_scanner_init (TagScanner *self)
{
     
     //create all
     self->pipeline = gst_pipeline_new ("pipeline");
     self->bus = gst_pipeline_get_bus (GST_PIPELINE (self->pipeline));
     self->filesrc = gst_element_factory_make ("gnomevfssrc", "source");
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
     gst_bus_set_flushing(self->bus,TRUE);
     //gst_bus_add_watch (self->bus, my_bus_callback, self);
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
     else{
#ifdef DEBUG
	  printf("%s\n",tag);
#endif

     }	 
	  

}
