/* xspf-reader.c */

#include "xspf-reader.h"
#include "pl-reader.h"
#include "tag-scanner.h"
#include <libxml/xmlreader.h>
#include <glib.h>
#include <string.h>


static void
xspf_reader_playlist_interface_init(PlaylistReaderInterface *iface);
static void 
process_xspf_tracks(xmlNode *nptr ,GList **list);
static void 
plist_xspf_add_file(xmlNode *nptr ,metadata *track);
static void 
foreach_xspf(gpointer data,gpointer user_data);
static const char *
xspf_mime_type(PlaylistReader *plist);

G_DEFINE_TYPE_WITH_CODE (XspfReader, xspf_reader, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (PLAYLIST_TYPE_READER,xspf_reader_playlist_interface_init));

#define XSPF_XMLNS "http://xspf.org/ns/0/"
#define CREATOR "musicplayer"
#define MIME_TYPE "application/xspf+xml"

struct _XspfReaderPrivate{
    
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	xmlNodePtr tracklist;
    
};



static gboolean
xspf_reader_read_list(PlaylistReader *plist,
                      gchar *location,
                      GList **list)
{

	xmlNode *nptr, *nptr2;
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
	xmlFreeDoc(doc);

	return TRUE;
}

static void 
process_xspf_tracks(xmlNode *nptr,
                    GList **list)
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

static void 
plist_xspf_add_file(xmlNode *nptr ,
                    metadata *track)
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


static gboolean
xspf_reader_write_list(PlaylistReader *plist,
                       gchar *location,
                       GList *list)
{
	XspfReader *self = XSPF_READER(plist);
	const xmlChar version[] = "1.0";
	const xmlChar playlist[] = "playlist";
	if(list != NULL)
	{
		self->priv->doc = xmlNewDoc(version);
		self->priv->rootnode = xmlNewNode(NULL,playlist);

		xmlSetProp(self->priv->rootnode, BAD_CAST "version", BAD_CAST "1");
		xmlSetProp(self->priv->rootnode, BAD_CAST "xmlns",BAD_CAST XSPF_XMLNS);

		xmlDocSetRootElement(self->priv->doc, self->priv->rootnode);
		xmlNewChild(self->priv->rootnode, NULL,BAD_CAST "creator",BAD_CAST CREATOR);
		self->priv->tracklist = xmlNewNode(NULL, BAD_CAST "trackList");
		xmlAddChild(self->priv->rootnode, self->priv->tracklist);


		g_list_foreach(list,foreach_xspf,self);


		xmlSaveFormatFileEnc(location, self->priv->doc, "UTF-8", 1);

		xmlFreeDoc(self->priv->doc);
		xmlCleanupParser();


	}
	return TRUE;

    
}
static void 
foreach_xspf(gpointer data,
             gpointer user_data)
{
	metadata *track = (metadata *) data;
	XspfReader *self = XSPF_READER(user_data);
	xmlNodePtr tracknode;
	xmlNodePtr location, title,artist;

	if(data != NULL)
	{
		tracknode = xmlNewNode(NULL,BAD_CAST "track");
		xmlAddChild(self->priv->tracklist, tracknode);

		location = xmlNewNode(NULL,BAD_CAST "location");

		xmlAddChild(location, xmlNewText((xmlChar *)track->uri));
		xmlAddChild(tracknode, location);

		if(track->artist)
		{

			title = xmlNewNode(NULL,BAD_CAST "title");
			artist =  xmlNewNode(NULL,BAD_CAST "creator");     

			xmlAddChild(title, xmlNewText((xmlChar *)track->title));
			xmlAddChild(artist, xmlNewText((xmlChar *)track->artist));

			xmlAddChild(tracknode, title);
			xmlAddChild(tracknode, artist);
		}
	}

	ts_metadata_free(track);
}
static void
xspf_reader_playlist_interface_init(PlaylistReaderInterface *iface)
{
	iface->playlist_reader_read_list=xspf_reader_read_list;
	iface->playlist_reader_write_list=xspf_reader_write_list;
	iface->playlist_reader_mime_supported=xspf_mime_type;
}

static const char *
xspf_mime_type(PlaylistReader *plist)
{
	return MIME_TYPE;
}

static void
xspf_reader_finalize (GObject *object)
{
	G_OBJECT_CLASS (xspf_reader_parent_class)->finalize (object);
	xmlCleanupParser();
}

static void
xspf_reader_class_init (XspfReaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (XspfReaderPrivate));

	object_class->finalize = xspf_reader_finalize;
}

static void
xspf_reader_init (XspfReader *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, XSPF_TYPE_READER, XspfReaderPrivate);
}

XspfReader*
xspf_reader_new (void)
{
	return g_object_new (XSPF_TYPE_READER, NULL);
}

