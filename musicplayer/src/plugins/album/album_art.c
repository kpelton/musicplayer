/* real-test.c */

#include "album_art.h"
#include "music-plugin.h"
#include "tag-scanner.h"
#include "utils.h"
#include <glib.h>
#include <libsoup/soup.h>       
#include <unistd.h>

#include <glib/gstdio.h>
#include <string.h>


static const char PLUGIN_NAME[] = "AlbumArt";
static const char DESC[] = "Show album art";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
//static gboolean is_configurable = FALSE;

struct {
    guint hash;
    AlbumArt *self;
    metadata *curr;
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

static void album_art_download_art(AsyncMsg *amsg, const char *msg, gsize msg_len);

static gboolean album_art_has_text(const gchar *text)
{
    if (text == NULL || *text == '\0')
        return FALSE;

    while (*text != '\0')
    {
        if (!g_ascii_isspace((guchar)*text))
            return TRUE;
        text++;
    }

    return FALSE;
}

static void album_art_hide(AlbumArt *self)
{
    gtk_image_clear(GTK_IMAGE(self->album));
    gtk_widget_hide(self->album);
}

static gboolean album_art_is_current(AsyncMsg *amsg)
{
    return amsg->self->mw != NULL && amsg->self->mw->currsong != NULL &&
           g_strcmp0(amsg->self->mw->currsong->artist, amsg->curr->artist) == 0 &&
           g_strcmp0(amsg->self->mw->currsong->album, amsg->curr->album) == 0;
}

static void album_art_free_async_msg(AsyncMsg *amsg)
{
    ts_metadata_free(amsg->curr);
    g_object_unref(amsg->self);
    g_free(amsg);
}

static gboolean album_art_load_file(AlbumArt *self, const gchar *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 48, 48, &error);
    if (pixbuf == NULL)
    {
        g_clear_error(&error);
        return FALSE;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(self->album), pixbuf);
    g_object_unref(pixbuf);
    gtk_widget_show(self->album);
    return TRUE;
}

typedef struct {
    gboolean in_medium_image;
    GString *image_url;
} AlbumArtXmlState;

static void album_art_xml_start_element(GMarkupParseContext *context,
                                        const gchar *element_name,
                                        const gchar **attribute_names,
                                        const gchar **attribute_values,
                                        gpointer user_data,
                                        GError **error)
{
    AlbumArtXmlState *state = user_data;
    guint i;

    (void)context;
    (void)error;

    if (g_strcmp0(element_name, "image") != 0)
        return;

    for (i = 0; attribute_names[i] != NULL; i++)
    {
        if (g_strcmp0(attribute_names[i], "size") == 0 &&
            g_strcmp0(attribute_values[i], "medium") == 0)
        {
            state->in_medium_image = TRUE;
            g_string_set_size(state->image_url, 0);
            return;
        }
    }
}

static void album_art_xml_text(GMarkupParseContext *context,
                               const gchar *text,
                               gsize text_len,
                               gpointer user_data,
                               GError **error)
{
    AlbumArtXmlState *state = user_data;

    (void)context;
    (void)error;

    if (state->in_medium_image)
        g_string_append_len(state->image_url, text, text_len);
}

static void album_art_xml_end_element(GMarkupParseContext *context,
                                      const gchar *element_name,
                                      gpointer user_data,
                                      GError **error)
{
    AlbumArtXmlState *state = user_data;

    (void)context;
    (void)error;

    if (state->in_medium_image && g_strcmp0(element_name, "image") == 0)
    {
        g_strstrip(state->image_url->str);
        state->in_medium_image = FALSE;
    }
}

static gchar *album_art_extract_image_url(const gchar *xml, gsize xml_len)
{
    static const GMarkupParser parser = {
        album_art_xml_start_element,
        album_art_xml_end_element,
        album_art_xml_text,
        NULL,
        NULL
    };
    AlbumArtXmlState state = { FALSE, g_string_new(NULL) };
    GMarkupParseContext *context;
    GError *error = NULL;
    gchar *image_url = NULL;

    context = g_markup_parse_context_new(&parser, 0, &state, NULL);
    if (g_markup_parse_context_parse(context, xml, xml_len, &error) &&
        g_markup_parse_context_end_parse(context, &error))
    {
        if (album_art_has_text(state.image_url->str))
            image_url = g_strdup(state.image_url->str);
    }

    g_clear_error(&error);
    g_markup_parse_context_free(context);
    g_string_free(state.image_url, TRUE);
    return image_url;
}

G_DEFINE_TYPE (AlbumArt, album_art, MUSIC_TYPE_PLUGIN);



MusicPluginDetails * 
get_details()
{   

	MusicPluginDetails *info;

	info = g_malloc0(sizeof(MusicPluginDetails));

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
    AsyncMsg *amsg = (AsyncMsg *)user_data;
    gchar *outputdir;
    const gchar *home = g_get_home_dir();
    GError *error = NULL;

    (void)session;

    outputdir = g_strdup_printf("%s/.musicplayer/art/%u", home, amsg->hash);
    if (msg->status_code == SOUP_STATUS_OK &&
        msg->response_body != NULL &&
        msg->response_body->length > 0 &&
        g_file_set_contents(outputdir,
                            msg->response_body->data,
                            msg->response_body->length,
                            &error))
    {
        if (album_art_is_current(amsg) &&
            !album_art_load_file(amsg->self, outputdir))
            album_art_hide(amsg->self);
    }
    else if (album_art_is_current(amsg))
    {
        album_art_hide(amsg->self);
    }

    g_clear_error(&error);
    g_free(outputdir);
    album_art_free_async_msg(amsg);
}

static void album_art_download_art(AsyncMsg *amsg, const char *msg, gsize msg_len)
{
    gchar *image_url;
    SoupMessage *smsg;
    SoupSession * session;

    image_url = album_art_extract_image_url(msg, msg_len);
    if (image_url == NULL)
    {
        if (album_art_is_current(amsg))
            album_art_hide(amsg->self);
        album_art_free_async_msg(amsg);
        return;
    }

    session = soup_session_new();
    smsg = soup_message_new ("GET", image_url);
    g_free(image_url);
    if (smsg == NULL)
    {
        if (album_art_is_current(amsg))
            album_art_hide(amsg->self);
        g_object_unref(session);
        album_art_free_async_msg(amsg);
        return;
    }

    soup_session_queue_message(session, smsg, album_art_got_image_response, amsg);
    
}

gboolean album_art_music_plugin_activate (MusicPlugin *self,MusicMainWindow *mw)
{
	AlbumArt * real = (AlbumArt *)self;
	real->mw = mw;
	g_object_ref(real->mw);
	real->id2 = g_signal_connect (mw->player, "new-file",
	                              G_CALLBACK(album_art_new_file),
	                              (gpointer)real);

	gchar *outputdir = NULL;
	const gchar *home = g_get_home_dir();

	outputdir = g_strdup_printf("%s/.musicplayer/art",home);


	if (!g_file_test(outputdir, G_FILE_TEST_IS_DIR))
		g_mkdir_with_parents(outputdir, 0700);
	g_free(outputdir);

	
	if (real->album == NULL)
		real->album = gtk_image_new();
	if (gtk_widget_get_parent(real->album) == NULL)
		gtk_box_pack_start (GTK_BOX (real->mw->mainhbox), real->album,
		                    TRUE, TRUE, 10);
	
	gtk_widget_show(real->album);
	if (real->mw->currsong)
	    album_art_new_file(NULL,real->mw->currsong,real);
	else
	    gtk_widget_hide(real->album);
	return TRUE;
}

static void album_art_new_file(GsPlayer *player,
                               metadata* p_track,gpointer user_data)
{
	AlbumArt * self = (AlbumArt *)user_data;
	SoupMessage *msg;
	SoupSession * session;
	const char url [] = "https://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=b25b959554ed76058ac220b7b2e0a026&";
	gchar *url_artist;
	gchar *url_album;
	gchar *url2;
	gchar *cache_key;
	gchar *outputdir;
	guint hash;
	const gchar *home = g_get_home_dir();
	AsyncMsg *amsg;

	(void)player;

	/* A missing album is a valid metadata state. Do not leave a placeholder
	 * image in the toolbar for tracks that have no album tag. */
	if (p_track == NULL ||
	    !album_art_has_text(p_track->artist) ||
	    !album_art_has_text(p_track->album))
	{
	    album_art_hide(self);
	    return;
	}

	gtk_image_set_from_icon_name(GTK_IMAGE(self->album), "image-missing",
	                            GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(self->album);

	/* Keep the existing cache key so artwork downloaded by older versions is
	 * still usable. */
	cache_key = g_strdup_printf("%s%s", p_track->artist, p_track->album);
	hash = g_str_hash(cache_key);
	g_free(cache_key);
	outputdir = g_strdup_printf("%s/.musicplayer/art/%u", home, hash);

	if (g_file_test(outputdir, G_FILE_TEST_EXISTS))
	{
	    if (album_art_load_file(self, outputdir))
	    {
	        g_free(outputdir);
	        return;
	    }
	    /* A partial/old cache entry should not permanently disable downloads. */
	    g_remove(outputdir);
	}
	g_free(outputdir);

	/* Query parameters must be escaped. Newer albums commonly contain '&',
	 * apostrophes, question marks, or non-ASCII characters. */
	url_artist = g_uri_escape_string(p_track->artist, NULL, FALSE);
	url_album = g_uri_escape_string(p_track->album, NULL, FALSE);
	url2 = g_strdup_printf("%sartist=%s&album=%s", url, url_artist, url_album);
	g_free(url_artist);
	g_free(url_album);

	amsg = g_malloc0(sizeof(AsyncMsg));
	amsg->hash = hash;
	amsg->self = g_object_ref(self);
	amsg->curr = ts_metadata_new();
	ts_metadata_copy(p_track, amsg->curr);
	session = soup_session_new();
	msg = soup_message_new ("GET", url2);
	g_free(url2);
	if (msg == NULL)
	{
	    album_art_hide(self);
	    g_object_unref(session);
	    album_art_free_async_msg(amsg);
	    return;
	}
	soup_session_queue_message(session, msg, album_art_got_xml_response, amsg);

}

void 
album_art_got_xml_response(SoupSession *session,
			  SoupMessage *msg,
			  gpointer user_data)
{
    AsyncMsg *amsg = (AsyncMsg *)user_data;

    (void)session;

    if (msg->status_code == SOUP_STATUS_OK && msg->response_body != NULL){
	album_art_download_art(amsg, msg->response_body->data,
	                       msg->response_body->length);
    }
    else{
	if (album_art_is_current(amsg))
	    album_art_hide(amsg->self);
	album_art_free_async_msg(amsg);
	
    }
}

gboolean album_art_music_plugin_deactivate ( MusicPlugin *user_data)
{
	AlbumArt * self = (AlbumArt *)user_data;

	if (self->mw != NULL && self->id2 != 0)
	{
		g_signal_handler_disconnect (G_OBJECT (self->mw->player),
		                             self->id2);
		self->id2 = 0;
	}

	if (self->mw != NULL)
	{
		g_object_unref(self->mw);
		self->mw = NULL;
	}
	if (self->album != NULL)
	{
		gtk_widget_destroy(self->album);
		self->album = NULL;
	}
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
