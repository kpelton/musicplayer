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

#define PLAYLIST_READER_GET_INTERFACE(inst) \
(G_TYPE_INSTANCE_GET_INTERFACE ((inst), PLAYLIST_TYPE_READER, PlaylistReaderInterface))


typedef struct _PlaylistReader PlaylistReader;
typedef struct _PlaylistReaderInterface PlaylistReaderInterface;

struct _PlaylistReaderInterface {

	GTypeInterface parent_iface;

	const gchar* (*playlist_reader_mime_supported) (PlaylistReader *self);
	gboolean (*playlist_reader_write_list) (PlaylistReader *self,gchar *location,GList * list);
	gboolean (*playlist_reader_read_list ) (PlaylistReader *self,gchar *location,GList **list);

};

GType playlist_reader_get_type (void);


gboolean playlist_reader_write_list (PlaylistReader *self,gchar *location,GList * list);
gboolean playlist_reader_read_list (PlaylistReader *self,gchar *location,GList **list);

G_END_DECLS

#endif /* _PLAYLIST_READER */
