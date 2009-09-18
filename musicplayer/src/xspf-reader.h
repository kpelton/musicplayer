/* xspf-reader.h */

#ifndef _XSPF_READER
#define _XSPF_READER

#include <glib-object.h>
#include "pl-reader.h"

G_BEGIN_DECLS

#define XSPF_TYPE_READER xspf_reader_get_type()

#define XSPF_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), XSPF_TYPE_READER, XspfReader))

#define XSPF_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), XSPF_TYPE_READER, XspfReaderClass))

#define XSPF_IS_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XSPF_TYPE_READER))

#define XSPF_IS_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), XSPF_TYPE_READER))
 
#define XSPF_READER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), XSPF_TYPE_READER, XspfReaderClass))

typedef struct _XspfReaderPrivate XspfReaderPrivate;
typedef struct {
  GObject parent;
  XspfReaderPrivate * priv;
} XspfReader;

typedef struct {
  GObjectClass parent_class;
} XspfReaderClass;

GType xspf_reader_get_type (void);

XspfReader* xspf_reader_new (void);
G_END_DECLS

#endif /* _XSPF_READER */
