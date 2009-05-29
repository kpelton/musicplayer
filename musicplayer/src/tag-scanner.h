/* tag-scanner.h */

#ifndef _TAG_SCANNER
#define _TAG_SCANNER

#include <glib-object.h>

#include <semaphore.h>
#include <gst/gst.h>


G_BEGIN_DECLS

#define TAG_TYPE_SCANNER tag_scanner_get_type()

#define TAG_SCANNER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TAG_TYPE_SCANNER, TagScanner))

#define TAG_SCANNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TAG_TYPE_SCANNER, TagScannerClass))

#define TAG_IS_SCANNER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TAG_TYPE_SCANNER))

#define TAG_IS_SCANNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TAG_TYPE_SCANNER))

#define TAG_SCANNER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TAG_TYPE_SCANNER, TagScannerClass))
typedef struct
{
     gchar *uri;
     gchar *title;
     gchar *artist;
     gchar *genre;
     gchar *album;
     gchar *codec;
     guint64 duration;
  
}metadata;

typedef struct {
     GObject parent;
     GstElement *pipeline;
     GstElement *filesrc;
     GstElement *dec;
     GstElement *fakesink;
     GstBus *bus;
     metadata *track;
     gboolean already_found;
} TagScanner;

typedef struct {
  GObjectClass parent_class;
} TagScannerClass;

GType tag_scanner_get_type (void);

TagScanner* tag_scanner_new (void);




metadata * ts_get_metadata(gchar * uri,TagScanner * self);
void ts_metadata_free(metadata *track);
metadata * ts_metadata_new();
metadata * ts_parse_file_name(gchar *metadata);
G_END_DECLS

#endif /* _TAG_SCANNER */
