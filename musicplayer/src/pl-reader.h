/* playlist-reader.h */

#ifndef _PLAYLIST_READER
#define _PLAYLIST_READER

#include <glib-object.h>

G_BEGIN_DECLS

#define PLAYLIST_TYPE_READER playlist_reader_get_type()

#define PLAYLIST_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLAYLIST_TYPE_READER, PlaylistReader))

#define PLAYLIST_IS_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLAYLIST_TYPE_READER))

#define PLAYLIST_READER_GET_INST(inst) \
  (G_TYPE_INSTANCE_GET_INTERFACE((inst), PLAYLIST_TYPE_READER, PlaylistReaderInterface))

typedef struct _PlayListReader PlaylistReader;

typedef struct {
  GTypeInterface parent_class;
    
    void (*do_action) (PlayListReader *self);

} PlaylistReaderInterface;

GType playlist_reader_get_type (void);

void playlist_reader_do_action(PlayListReader *self);

G_END_DECLS

#endif /* _PLAYLIST_READER */
