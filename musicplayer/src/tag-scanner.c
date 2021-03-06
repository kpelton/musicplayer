/* tag-scanner.c */

#include "tag-scanner.h"
#include "music-queue.h"
#include <string.h>
#include <glib.h>
//#define DEBUG



G_DEFINE_TYPE (TagScanner, tag_scanner, G_TYPE_OBJECT)



static void ts_event_loop(TagScanner * self, GstBus *bus);

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


static void 
ts_event_loop  (TagScanner * self, GstBus *bus)
{
	GstMessage *message;
	gboolean val = FALSE;
	message = gst_bus_timed_pop  (bus,500); 
	while( val != TRUE )
	{

		if(message != NULL)
		{
			val = my_bus_callback(bus,message,self);

			gst_message_unref(message);

		}
		if(!val)
			message = gst_bus_timed_pop  (bus,500); 
	}


}


metadata * 
ts_get_metadata(gchar * uri,TagScanner * self)
{
	GstQuery *query;
	gboolean res;
	query = gst_query_new_duration (GST_FORMAT_TIME);

	self->track = ts_metadata_new ();

	//create all
	self->pipeline = gst_pipeline_new ("pipeline");

	self->dec  = gst_element_factory_make ("uridecodebin", "decodebin");
	gst_bin_add (GST_BIN (self->pipeline), self->dec);
	g_object_set (G_OBJECT (self->dec), "uri", uri, NULL);

	self->fakesink = gst_element_factory_make ("fakesink", NULL);
	gst_bin_add (GST_BIN (self->pipeline), self->fakesink);
	g_signal_connect (self->dec, "pad-added", G_CALLBACK (cb_newpad),self->fakesink);
	self->bus = gst_pipeline_get_bus (GST_PIPELINE (self->pipeline));
	//signals




	//connect everything






	self->already_found = FALSE;


//g_object_set (self->dec, "uri", uri, NULL);
	//gst_bus_add_watch (self->bus, my_bus_callback, self);
	gst_element_set_state (self->pipeline, GST_STATE_PAUSED);


	ts_event_loop(self,self->bus);


	
	
	res = gst_element_query (self->pipeline, query);
	if (res) 
	{
  		gst_query_parse_duration (query, NULL,&(self->track->duration));
	}
#ifdef DEBUG   
	else 
	{
  		g_print ("duration query failed...\n");
	}
#endif
	//unref
	gst_query_unref (query);
	gst_object_unref (self->bus);
	gst_element_set_state (self->pipeline, GST_STATE_NULL);
	gst_object_unref(self->pipeline);

	self->pipeline = NULL;
	return self->track;


}

void ts_metadata_copy(metadata *src,metadata *dst){
	if(src){
		if(src->uri)
		    dst->uri = g_strdup(src->uri);
		if(src->title)
		    dst->title = g_strdup(src->title);
		if(src->artist)
		    dst->artist = g_strdup(src->artist);
		if(src->genre)
		    dst->genre = g_strdup(src->genre);
		if(src->album)
		    dst->album = g_strdup(src->album);
		if(src->codec)
		    dst->codec = g_strdup(src->codec);
		dst->duration  = src->duration;
	}


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
	gint percent = 0;



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

		case GST_MESSAGE_EOS:  
			return TRUE;
			break;
		case GST_MESSAGE_BUFFERING: 

			gst_message_parse_buffering (message, &percent);
			g_print ("Buffering (%u percent done)", percent);
			break;

		case GST_MESSAGE_TAG:



#ifdef DEBUG
			printf("GOT TAG\n");
#endif
				gst_message_parse_tag(message,&list); 

			if(!gst_tag_list_is_empty(list)){
				gst_tag_list_foreach (list,gst_new_tags,self->track);
			}

			gst_tag_list_free (list);


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
           gpointer    data)
{
	GstPad *sinkpad;
	GstElement *fakesink = (GstElement *) data;

  sinkpad = gst_element_get_static_pad (fakesink, "sink");
  if (!gst_pad_is_linked (sinkpad)) {
    if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK)
      g_error ("Failed to link pads!");
  }
  gst_object_unref (sinkpad);
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
