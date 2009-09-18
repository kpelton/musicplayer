/* m3u-reader.c */

#include "m3u-reader.h"
#include "pl-reader.h"
static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface);

G_DEFINE_TYPE_WITH_CODE (M3uReader, m3u_reader, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PLAYLIST_TYPE_READER,m3u_reader_playlist_interface_init));

static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface)
{

  //iface->playlist_reader_read_list=xspf_reader_read_list;
  //iface->playlist_reader_write_list=xspf_reader_write_list;
  //iface->playlist_reader_mime_supported=xspf_mime_type;

}
static void
m3u_reader_finalize (GObject *object)
{
  G_OBJECT_CLASS (m3u_reader_parent_class)->finalize (object);
}

static void
m3u_reader_class_init (M3uReaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);


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

