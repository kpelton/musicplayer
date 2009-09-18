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
pl_reader_get_type (void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (PlayListReaderInterface),
        playlist_reader_base_init,   /* base_init */
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "PlayListReader",
                                           &info, 0);
    }

  return iface_type;
}

void
playlist_reader_do_action (PlayListReader *self)
{
  g_return_if_fail (PLAY_LIST_IS_READER_(self));

  PLAYLIST_READER_GET_INTERFACE (self)->do_action (self);
}
