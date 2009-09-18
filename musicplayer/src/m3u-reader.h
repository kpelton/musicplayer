/* m3u-reader.h */

#ifndef _M3U_READER
#define _M3U_READER

#include <glib-object.h>
#include "pl-reader.h"

G_BEGIN_DECLS

#define M3U_TYPE_READER m3u_reader_get_type()

#define M3U_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), M3U_TYPE_READER, M3uReader))

#define M3U_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), M3U_TYPE_READER, M3uReaderClass))

#define M3U_IS_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), M3U_TYPE_READER))

#define M3U_IS_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), M3U_TYPE_READER))

#define M3U_READER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), M3U_TYPE_READER, M3uReaderClass))

typedef struct _M3uReaderPrivate M3uReaderPrivate;
typedef struct {
  GObject parent;
  M3uReaderPrivate *priv;
} M3uReader;

typedef struct {
  GObjectClass parent_class;
} M3uReaderClass;

GType m3u_reader_get_type (void);

M3uReader* m3u_reader_new (void);

G_END_DECLS

#endif /* _M3U_READER */
