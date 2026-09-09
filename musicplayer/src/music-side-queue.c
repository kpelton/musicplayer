/* music-side-queue.c */

#include "music-side-queue.h"
#include <glib.h>

enum
{
	QUEUE_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

struct _MusicSideQueuePrivate {
	int dummy;
	GList *queue;
	guint size;
};

G_DEFINE_TYPE_WITH_PRIVATE (MusicSideQueue, music_side_queue, G_TYPE_OBJECT)

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
	if(object){
		MusicSideQueue *self = MUSIC_SIDE_QUEUE(object) ;
		g_list_foreach (self->priv->queue,(GFunc)g_free,NULL);
		g_list_free(self->priv->queue);
	}
	G_OBJECT_CLASS (music_side_queue_parent_class)->finalize (object);
}

static void
music_side_queue_class_init (MusicSideQueueClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = music_side_queue_get_property;
	object_class->set_property = music_side_queue_set_property;
	object_class->dispose = music_side_queue_dispose;
	object_class->finalize = music_side_queue_finalize;

	signals[QUEUE_CHANGED] = g_signal_new ("queue-changed",
	                                      G_TYPE_FROM_CLASS (klass),
	                                      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                                      0,
	                                      NULL,
	                                      NULL,
	                                      g_cclosure_marshal_VOID__VOID,
	                                      G_TYPE_NONE,
	                                      0);
}

static void
music_side_queue_init (MusicSideQueue *self)
{
	self->priv = music_side_queue_get_instance_private (self);
	self->priv->queue = NULL;
	self->priv->size = 0;
}
void
music_side_queue_enqueue(MusicSideQueue *self,
                         guint id)
{
	guint *newid = NULL;

	newid = g_malloc(sizeof(guint));
	*newid = id;
	self->priv->queue = g_list_prepend(self->priv->queue,newid); 
	g_signal_emit (self, signals[QUEUE_CHANGED], 0);

}

gboolean
music_side_queue_contains(MusicSideQueue *self,
                          guint id)
{
	return music_side_queue_get_position(self, id) > 0;

}

guint
music_side_queue_get_position(MusicSideQueue *self,
                              guint id)
{
	GList *node;
	guint position = 1;

	/* New entries are prepended, while dequeue removes the last entry. Walk
	 * backwards so position 1 is the next song to play. */
	for (node = g_list_last(self->priv->queue);
	     node != NULL;
	     node = node->prev, position++)
	{
		if (node->data && *((guint *) node->data) == id)
			return position;
	}

	return 0;

}

void
music_side_queue_remove(MusicSideQueue *self,
                        guint id)
{
	GList *node;
	gboolean removed = FALSE;

	for (node = self->priv->queue; node != NULL; )
	{
		GList *next = node->next;

		if (node->data && *((guint *) node->data) == id)
		{
			g_free(node->data);
			self->priv->queue = g_list_delete_link(self->priv->queue, node);
			removed = TRUE;
		}

		node = next;
	}

	if (removed)
		g_signal_emit (self, signals[QUEUE_CHANGED], 0);

}

guint 
music_side_queue_dequeue(MusicSideQueue *self)
{
	guint *id = NULL;
	GList *last = NULL;
	guint retid = 0;
	if(self->priv->queue != NULL)
	{
		last =  g_list_last (self->priv->queue);
		id = (guint*) last->data;

		if(last->prev && last->prev->next == last)
			last->prev->next= NULL;
		else
			self->priv->queue = NULL;

		if(id)
			retid = *id;
		
		g_free(last->data);
		g_list_free1(last);
		g_signal_emit (self, signals[QUEUE_CHANGED], 0);


	}



	return retid;
}




MusicSideQueue*
music_side_queue_new (void)
{
	return g_object_new (MUSIC_TYPE_SIDE_QUEUE, NULL);
}
