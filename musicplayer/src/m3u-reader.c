/* m3u-reader.c */

#include "m3u-reader.h"
#include "pl-reader.h"
#include "tag-scanner.h"
#include <gio/gio.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <glib.h>
#include <string.h>

#define MIME_TYPE "audio/x-mpegurl"


static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface);

static const gchar *
m3u_reader_mime_type();

//private vars
struct _M3uReaderPrivate
{

};
//end privates vars


G_DEFINE_TYPE_WITH_CODE (M3uReader, m3u_reader, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PLAYLIST_TYPE_READER,m3u_reader_playlist_interface_init));

static gboolean
m3u_reader_read_list(PlaylistReader *plist,
                      gchar *location,
                      GList **list)
{
    M3uReader *self = M3U_READER(plist);
    gssize count=0;
    GError *err;
    gchar *buffer;
    gchar **lines =NULL;
    GFileInfo *info;
    GFile *file;
    int i;
    TagScanner *ts;
    gchar *uri;
    gchar *escaped;
    gchar *newuri;
    metadata *md;
    gchar *uribeg; 
    uri = g_strdup(location);
    uribeg= uri;
    gchar *lineptr;


    //get the right directory for the uri
    for(uri=uri+strlen(uri); *uri != '/'; uri--);

        *(uri)='\0';

    uri = uribeg;
        
    g_file_get_contents               (gnome_vfs_get_local_path_from_uri (location),
                                      &buffer,
                                      &count,
                                      &err);

    if(count >0)
    {
        lines = g_strsplit(buffer,"\n",-1);
        if(lines){
            ts = tag_scanner_new ();
            for(i=0; lines[i] != NULL; i++)
            {
                if( *lines[i] != '\0' && *lines[i] != '#' && *lines[i] != '\r')
                {
                    if(lines[i][strlen(lines[i])-1] == '\r' || lines[i][strlen(lines[i])-1] == '\n')
                        lines[i][strlen(lines[i])-1] = '\0';  
                    
                    //get rid of the \n at the start and  at the end
                    lines[i][strlen(lines[i])] = '\0';   
    
                    
                    lineptr = lines[i];
                    
                    if(*lineptr == '\n')
                        lineptr++;

                
                    escaped = g_uri_escape_string(lineptr,NULL,TRUE);
                    newuri = g_malloc(sizeof(gchar) *strlen(escaped)+strlen(uri)+10);
                    g_snprintf(newuri,strlen(escaped)+strlen(uri)+10,"%s/%s",uri,escaped);

                    if( g_file_test(gnome_vfs_get_local_path_from_uri (newuri),
                                    G_FILE_TEST_EXISTS))
                    {
                        md=ts_get_metadata (newuri,ts);

                        if(md) //has metadata
                        {
                            md->uri = strdup(newuri);
                            *list = g_list_append(*list,md);
                        }
                        else //no metadata copy uri
                        {
                         md = ts_metadata_new ();
                         md->uri = strdup(newuri);
                         *list = g_list_append(*list,md);
                        }
                    }
                 g_free(escaped);
                 g_free(newuri);   
                }
                
            }
            g_free(uri);
            g_strfreev(lines); 
            g_object_unref(ts);
             return TRUE;
        }
    }
     
    return FALSE;
}


static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface)
{

  iface->playlist_reader_read_list=m3u_reader_read_list;
  //iface->playlist_reader_write_list=xspf_reader_write_list;
  iface->playlist_reader_mime_supported=m3u_reader_mime_type;

}




static const gchar *
m3u_reader_mime_type()
{
    return MIME_TYPE;
}

static void
m3u_reader_finalize (GObject *object)
{
    M3uReader *self = M3U_READER(object);


    
  G_OBJECT_CLASS (m3u_reader_parent_class)->finalize (object);
}

static void
m3u_reader_class_init (M3uReaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

   //g_type_class_add_private (klass, sizeof (M3uReaderPrivate));  
  object_class->finalize = m3u_reader_finalize;
}

static void
m3u_reader_init (M3uReader *self)
{
   
}

M3uReader*
m3u_reader_new (void)
{
  return g_object_new (M3U_TYPE_READER, NULL);
}

