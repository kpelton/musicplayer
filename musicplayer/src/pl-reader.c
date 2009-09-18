#include "pl-reader.h"
static void
playlist_reader_base_init (gpointer g_class)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      /* add properties and signals to the interface here */

      is_initialized = TRUE;
    }
}

GType
playlist_reader_get_type (void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (PlaylistReaderInterface),
        playlist_reader_base_init,   /* base_init */
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "PlaylistReader",
                                           &info, 0);
    }

  return iface_type;
}


gboolean 
playlist_reader_write_list (PlaylistReader *self,gchar *location,GList * list)
{
  g_return_if_fail (PLAYLIST_IS_READER(self));

  return PLAYLIST_READER_GET_INTERFACE (self)->playlist_reader_write_list (self,
                                                                          location,
                                                                          list);
}
gboolean 
playlist_reader_read_list (PlaylistReader *self,gchar *location,GList **list)
{
  g_return_if_fail (PLAYLIST_IS_READER(self));

  return PLAYLIST_READER_GET_INTERFACE (self)->playlist_reader_read_list (self,
                                                                          location,
                                                                          list);
}

