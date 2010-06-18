
/* music-side-queue.h */

#ifndef _MUSIC_SIDE_QUEUE
#define _MUSIC_SIDE_QUEUE

#include <glib-object.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_SIDE_QUEUE music_side_queue_get_type()

#define MUSIC_SIDE_QUEUE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_SIDE_QUEUE, MusicSideQueue))

#define MUSIC_SIDE_QUEUE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_SIDE_QUEUE, MusicSideQueueClass))

#define MUSIC_IS_SIDE_QUEUE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_SIDE_QUEUE))

#define MUSIC_IS_SIDE_QUEUE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_SIDE_QUEUE))

#define MUSIC_SIDE_QUEUE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_SIDE_QUEUE, MusicSideQueueClass))

typedef struct _MusicSideQueuePrivate MusicSideQueuePrivate;

typedef struct {
  GObject parent;
	MusicSideQueuePrivate *priv;
} MusicSideQueue;

typedef struct {
  GObjectClass parent_class;
} MusicSideQueueClass;

GType music_side_queue_get_type (void);

MusicSideQueue* music_side_queue_new (void);

G_END_DECLS

#endif /* _MUSIC_SIDE_QUEUE */