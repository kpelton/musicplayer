/* xspf-reader.c */

#include "xspf-reader.h"
#include "pl-reader.h"

static void 
do_action(XspfReader *self);

G_DEFINE_TYPE_WITH_CODE (XspfReader, xspf_reader, G_TYPE_OBJECT,
                           G_IMPLEMENT_INTERFACE (PLAYLIST_TYPE_READER,
                                                ));


static void
xspf_reader_playlist_interface_init(PlaylistReaderInterface *iface)
{
  iface->do_action = do_action;
}

static void 
do_action(XspfReader *self)
{
  g_print ("xspf implementation of playlist interface Action: 0x%x.\n",
           self->instance_member);
}

static void
xspf_reader_finalize (GObject *object)
{
  G_OBJECT_CLASS (xspf_reader_parent_class)->finalize (object);
}

static void
xspf_reader_class_init (XspfReaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);


  object_class->finalize = xspf_reader_finalize;
}

static void
xspf_reader_init (XspfReader *self)
{
    
}

XspfReader*
xspf_reader_new (void)
{
  return g_object_new (XSPF_TYPE_READER, NULL);
}

