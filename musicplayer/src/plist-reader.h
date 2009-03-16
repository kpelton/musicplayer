/* plist-reader.h */

#ifndef _PLIST_READER
#define _PLIST_READER

#include <glib-object.h>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <glib.h>
#include "tag-scanner.h"
#define XSPF_XMLNS "http://xspf.org/ns/0/"
#define CREATOR "musicplayer"
G_BEGIN_DECLS



#define PLIST_TYPE_READER plist_reader_get_type()

#define PLIST_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLIST_TYPE_READER, PlistReader))

#define PLIST_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), PLIST_TYPE_READER, PlistReaderClass))

#define PLIST_IS_READER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLIST_TYPE_READER))

#define PLIST_IS_READER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLIST_TYPE_READER))

#define PLIST_READER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLIST_TYPE_READER, PlistReaderClass))

typedef struct {
  GObject parent;

     xmlDocPtr doc;
     xmlNodePtr rootnode;
     xmlNodePtr tracklist;
} PlistReader;

typedef struct {
  GObjectClass parent_class;
} PlistReaderClass;

GType plist_reader_get_type (void);

PlistReader* plist_reader_new (void);

gboolean plist_reader_write_list(gchar *location,GList * list,PlistReader *self);
gboolean plist_xspf_read(gchar *location,GList **list,PlistReader *self);

G_END_DECLS

#endif /* _PLIST_READER */
