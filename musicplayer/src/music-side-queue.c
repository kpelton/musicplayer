/* music-side-queue.c */

#include "music-side-queue.h"
#include <glib.h>

G_DEFINE_TYPE (MusicSideQueue, music_side_queue, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MUSIC_TYPE_SIDE_QUEUE, MusicSideQueuePrivate))



struct _MusicSideQueuePrivate {
    int dummy;
	GList *queue;
	guint size;
};

static void
music_side_queue_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
music_side_queue_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
music_side_queue_dispose (GObject *object)
{
  G_OBJECT_CLASS (music_side_queue_parent_class)->dispose (object);
}

static void
music_side_queue_finalize (GObject *object)
{
  G_OBJECT_CLASS (music_side_queue_parent_class)->finalize (object);
}

static void
music_side_queue_class_init (MusicSideQueueClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MusicSideQueuePrivate));

  object_class->get_property = music_side_queue_get_property;
  object_class->set_property = music_side_queue_set_property;
  object_class->dispose = music_side_queue_dispose;
  object_class->finalize = music_side_queue_finalize;
}

static void
music_side_queue_init (MusicSideQueue *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, MUSIC_TYPE_SIDE_QUEUE, 
						  MusicSideQueuePrivate);
	self->priv->queue = NULL;
	self->priv->size = 0;
}

MusicSideQueue*
music_side_queue_new (void)
{
  return g_object_new (MUSIC_TYPE_SIDE_QUEUE, NULL);
}