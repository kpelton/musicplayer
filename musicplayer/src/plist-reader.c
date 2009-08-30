/* plist-reader.c */

#include "plist-reader.h"
#include "tag-scanner.h"
#include <libxml/xmlreader.h>
#include <glib.h>
#include <string.h>

G_DEFINE_TYPE (PlistReader, plist_reader, G_TYPE_OBJECT)

static void foreach_xspf(gpointer data,gpointer user_data);
static void plist_xspf_add_file(xmlNode *nptr ,metadata *track);

static void process_xspf_tracks(xmlNode *nptr ,GList **list);
static void plist_reader_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)



{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
plist_reader_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
plist_reader_dispose (GObject *object)
{
  

  PlistReader *self = (PlistReader *) object;
  

  
    xmlCleanupParser();

    G_OBJECT_CLASS (plist_reader_parent_class)->dispose (object);

}
gboolean plist_xspf_read(gchar *location,GList **list,PlistReader *self)
{
     int ret;
     metadata *track = NULL;
     const xmlChar* name,*value;
     xmlNode *nptr, *nptr2, *nptr3;
     xmlDocPtr doc;
     doc = xmlRecoverFile(location);
     if (doc == NULL)
	  return FALSE;
     //playlist
    for (nptr = doc->children; nptr != NULL; nptr = nptr->next) 
    {
	 if (nptr->type == XML_ELEMENT_NODE &&
            !xmlStrcmp(nptr->name, (xmlChar *)"playlist"))
	 {
	      for (nptr2 = nptr->children; nptr2 != NULL; nptr2 = nptr2->next)
	      {
		   if (nptr2->type == XML_ELEMENT_NODE &&
		       !xmlStrcmp(nptr2->name, (xmlChar *)"trackList")){
			process_xspf_tracks(nptr2,list);
			
		   }
	    }
	 }
    }


}

static void process_xspf_tracks(xmlNode *nptr ,GList **list)
{
     metadata *track = NULL;


     for (nptr=nptr->children; nptr != NULL; nptr = nptr->next) {
	  //each track
	  if (nptr->type == XML_ELEMENT_NODE &&
	      !xmlStrcmp(nptr->name, (xmlChar *)"track")) {
	       track = ts_metadata_new();
	       plist_xspf_add_file(nptr,track);
	       if(track->uri)
		    *list = g_list_append(*list,track);
	       else
		    ts_metadata_free(track);
	  }

     }

}
static void plist_xspf_add_file(xmlNode *nptr ,metadata *track)
{
     const gchar *name;

     for (nptr=nptr->children; nptr != NULL; nptr = nptr->next) {
	  //each part of the track
	  name = (const gchar *)xmlNodeGetContent(nptr);
	  if (nptr->type == XML_ELEMENT_NODE &&
	      !xmlStrcmp(nptr->name,(xmlChar *)"location")) {
	       track->uri = g_malloc(strlen(name)+1);
	       strcpy(track->uri,name);
	  }
	  if (nptr->type == XML_ELEMENT_NODE &&
	      !xmlStrcmp(nptr->name, (xmlChar *)"title")) {
	       track->title = g_malloc(strlen(name)+1);
	       strcpy(track->title,name);
	  }
	  if (nptr->type == XML_ELEMENT_NODE &&
	      !xmlStrcmp(nptr->name,(xmlChar *)"creator")) {
	  
	       track->artist = g_malloc(strlen(name)+1);
	       strcpy(track->artist,name);
	  }
     }
}




gboolean plist_reader_write_list(gchar *location,GList * list,PlistReader *self)
{
     if(list != NULL)
     {
     self->doc = xmlNewDoc("1.0");
     self->rootnode = xmlNewNode(NULL,"playlist");

     xmlSetProp(self->rootnode, (xmlChar *)"version", (xmlChar *)"1");
     xmlSetProp(self->rootnode, (xmlChar *)"xmlns", (xmlChar *)XSPF_XMLNS);

     xmlDocSetRootElement(self->doc, self->rootnode);
     xmlNewChild(self->rootnode, NULL,"creator",CREATOR);
     self->tracklist = xmlNewNode(NULL, (xmlChar *)"trackList");
     xmlAddChild(self->rootnode, self->tracklist);
     
     
     g_list_foreach(list,foreach_xspf,self);
	  
     
     xmlSaveFormatFileEnc(location, self->doc, "UTF-8", 1);

     xmlFreeDoc(self->doc);
     xmlCleanupParser();

     
    }
     return TRUE;
 
}

static void foreach_xspf(gpointer data,gpointer user_data)
{
     metadata *track = (metadata *) data;
     PlistReader *self = (PlistReader *) user_data;
     xmlNodePtr tracknode ,node;
     xmlNodePtr location, title,artist;
     
     if(data != NULL)
     {
	  tracknode = xmlNewNode(NULL,"track");
	  xmlAddChild(self->tracklist, tracknode);
	   
	  //need to add album

	  
	  location = xmlNewNode(NULL,"location");


	  xmlAddChild(location, xmlNewText((xmlChar *)track->uri));
	  xmlAddChild(tracknode, location);
		      

	  if(track->artist)
	  {
	       
	  title = xmlNewNode(NULL,"title");
	  artist =  xmlNewNode(NULL,"creator");     
	  
	  xmlAddChild(title, xmlNewText((xmlChar *)track->title));
	  xmlAddChild(artist, xmlNewText((xmlChar *)track->artist));

	  xmlAddChild(tracknode, title);
	  xmlAddChild(tracknode, artist);


	  }
	   
     }
	       
     

      ts_metadata_free(track);
}
static void
plist_reader_finalize (GObject *object)
{
  G_OBJECT_CLASS (plist_reader_parent_class)->finalize (object);
}

static void
plist_reader_class_init (PlistReaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);


  object_class->get_property = plist_reader_get_property;
  object_class->set_property = plist_reader_set_property;
  object_class->dispose = plist_reader_dispose;
  object_class->finalize = plist_reader_finalize;
}

static void
plist_reader_init (PlistReader *self)
{
     self->doc = NULL;
     self->rootnode = NULL;
     self->tracklist = NULL;
     
}

PlistReader*
plist_reader_new (void)
{
  return g_object_new (PLIST_TYPE_READER, NULL);
}
