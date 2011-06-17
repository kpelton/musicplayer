/* real-test.c */

#include "album_art.h"
#include "music-plugin.h"
#include "tag-scanner.h"
#include "utils.h"
#include <glib.h>
#include <libsoup/soup.h>       
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib/gstdio.h>


static const char PLUGIN_NAME[] = "AlbumArt";
static const char DESC[] = "Show album art";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
//static gboolean is_configurable = FALSE;

struct {
    guint hash;
    AlbumArt *self;
}typedef AsyncMsg;



gboolean 
album_art_music_plugin_activate ( MusicPlugin  *self,MusicMainWindow *mw);

gboolean 
album_art_music_plugin_deactivate ( MusicPlugin *self);

MusicPluginDetails * 
album_art_get_info(MusicPlugin  *parent);

static void album_art_new_file(GsPlayer *player,
                               metadata* p_track,gpointer user_data);


void 
album_art_got_xml_response(SoupSession *session,
			  SoupMessage *msg,
			  gpointer user_data);

void 
album_art_got_image_response(SoupSession *session,
			  SoupMessage *msg,
			  gpointer user_data);

static void album_art_download_art(AsyncMsg *amsg,const char *msg);

G_DEFINE_TYPE (AlbumArt, album_art, MUSIC_TYPE_PLUGIN);



MusicPluginDetails * 
get_details()
{   

	MusicPluginDetails *info;

	info = g_malloc(sizeof(MusicPluginDetails));

	info->name = g_strdup(PLUGIN_NAME);
	info->desc = g_strdup(DESC);
	info->copyright = g_strdup(COPYRIGHT);
	info->website = g_strdup(WEBSITE);
	info->is_configurable = FALSE;

	return info;


}
void 
album_art_got_image_response(SoupSession *session,
			  SoupMessage *msg,
			  gpointer user_data)
{
    AsyncMsg * amsg = (AsyncMsg *)user_data;
    const gchar *home;
    
    gchar *outputdir=NULL;
    int file;
    int retval;

    if (msg->status_code == 200){
	home = g_getenv ("HOME");
	outputdir = g_strdup_printf("%s/.musicplayer/art/%u",home,amsg->hash);
	file = open(outputdir,O_WRONLY |O_CREAT,S_IRUSR|S_IWUSR);
	retval = write(file,msg->response_body->data,msg->response_body->length);
	
	close(file);
	if (retval >0)
	    gtk_image_set_from_pixbuf(GTK_IMAGE(amsg->self->album),
				      gdk_pixbuf_new_from_file_at_size (
								outputdir,64,64,NULL));
	g_free(outputdir);
    }
    g_free(amsg);
    
}

static void album_art_download_art(AsyncMsg *amsg,const char *msg)
{
    char *start;
    char *end;
    char startstr[] = "<image size=\"medium\">";
    int comblen = strlen(startstr);
    SoupMessage *smsg;
    SoupSession * session;

 
    
    
    
    start = strstr(msg,startstr);

    if (start != NULL){
	end = strstr(start,"</image>");
	*end = '\0';
	start+=comblen;
	session = soup_session_async_new();
	smsg = soup_message_new ("GET",start);
	soup_session_queue_message(session,smsg,album_art_got_image_response,amsg);
    }else{
	g_free(amsg);
    }
    
}

gboolean album_art_music_plugin_activate (MusicPlugin *self,MusicMainWindow *mw)
{
	AlbumArt * real = (AlbumArt *)self;
	real->mw = mw;
	g_object_ref(real->mw);
	real->id2 = g_signal_connect (mw->player, "new-file",
	                              G_CALLBACK(album_art_new_file),
	                              (gpointer)real);

	gchar *outputdir=NULL;
	const gchar *home;
	home = g_getenv ("HOME");

	outputdir = g_strdup_printf("%s/.musicplayer/art",home);


	if (!g_file_test(outputdir,G_FILE_TEST_EXISTS))
	{
		g_mkdir(outputdir,0700);
	}
	g_free(outputdir);

	
	  gtk_box_pack_start (GTK_BOX (real->mw->mainhbox), real->album, TRUE, TRUE,10);
	    
	  gtk_widget_show(real->album);
	return TRUE;
}

static void album_art_new_file(GsPlayer *player,
                               metadata* p_track,gpointer user_data)
{
	AlbumArt * self = (AlbumArt *)user_data;
	SoupMessage *msg;
	SoupSession * session;
	const char url [] = "http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=b25b959554ed76058ac220b7b2e0a026&";
	char url2[5000];
	char buffer[5000];
	char *outputdir;
	guint hash;
	const gchar *home;
	home = g_getenv ("HOME");
	AsyncMsg *amsg;



	gtk_image_clear (GTK_IMAGE(self->album));
	
	if(p_track->artist && p_track->album)   
	    {
		snprintf(buffer,5000,"%s%s",p_track->artist,p_track->album);
		hash = g_str_hash(buffer);

		amsg = g_malloc(sizeof(AsyncMsg));
		amsg->hash = hash;
		amsg->self = self;
		outputdir = g_strdup_printf("%s/.musicplayer/art/%u",home,hash);
		if (g_file_test(outputdir,G_FILE_TEST_EXISTS)){
		    gtk_image_set_from_pixbuf(GTK_IMAGE(self->album),
					      gdk_pixbuf_new_from_file_at_size (
										outputdir,64,64,NULL));
		    }
		    else{
			snprintf(url2,5000,"%sartist=%s&album=%s",url,p_track->artist,p_track->album);
			session = soup_session_sync_new();
			msg = soup_message_new ("GET",url2);
			soup_session_queue_message(session,msg,album_art_got_xml_response,amsg);
		    }
              }

}

void 
album_art_got_xml_response(SoupSession *session,
			  SoupMessage *msg,
			  gpointer user_data)
{
    AsyncMsg *amsg = (AsyncMsg *)user_data;

    if (msg->status_code == 200){
	album_art_download_art(amsg,msg->response_body->data);
    }
    else{
	g_free(amsg);
    }
}

gboolean album_art_music_plugin_deactivate ( MusicPlugin *user_data)
{
	AlbumArt * self = (AlbumArt *)user_data;

	g_signal_handler_disconnect (G_OBJECT (self->mw->player),
	                             self->id2);

	g_object_unref(self->mw);
	gtk_widget_destroy(self->album);
	return TRUE;
}

GType 
register_music_plugin()
{
	return album_art_get_type();
}


static void
album_art_dispose (GObject *object)
{

	G_OBJECT_CLASS (album_art_parent_class)->dispose (object);

}

static void
album_art_finalize (GObject *object)
{


	G_OBJECT_CLASS (album_art_parent_class)->finalize (object);
}
static void
album_art_init (AlbumArt *self)
{
    self->album = gtk_image_new();
}
static void
album_art_class_init (AlbumArtClass *klass)
{
	MusicPluginClass  *class = MUSIC_PLUGIN_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/* implement pure virtual class function. */
	class->music_plugin_activate=album_art_music_plugin_activate;
	class->music_plugin_deactivate=album_art_music_plugin_deactivate;


	object_class->dispose = album_art_dispose;
	object_class->finalize = album_art_finalize;


}

AlbumArt*
album_art_new (void)
{
	return g_object_new (ALBUM_TYPE_ART, NULL);
}

