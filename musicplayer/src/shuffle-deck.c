#include "shuffle-deck.h"

struct _MusicShuffleDeck
{
	GList *ids;
};

MusicShuffleDeck *
music_shuffle_deck_new (void)
{
	return g_new0(MusicShuffleDeck, 1);
}

void
music_shuffle_deck_free (MusicShuffleDeck *deck)
{
	if (deck == NULL)
		return;

	g_list_free(deck->ids);
	g_free(deck);
}

void
music_shuffle_deck_clear (MusicShuffleDeck *deck)
{
	g_return_if_fail(deck != NULL);

	g_list_free(deck->ids);
	deck->ids = NULL;
}

void
music_shuffle_deck_add (MusicShuffleDeck *deck,
                        guint id)
{
	g_return_if_fail(deck != NULL);

	if (id == 0 || g_list_find(deck->ids, GUINT_TO_POINTER(id)) != NULL)
		return;

	deck->ids = g_list_append(deck->ids, GUINT_TO_POINTER(id));
}

void
music_shuffle_deck_remove (MusicShuffleDeck *deck,
                           guint id)
{
	g_return_if_fail(deck != NULL);

	deck->ids = g_list_remove(deck->ids, GUINT_TO_POINTER(id));
}

void
music_shuffle_deck_shuffle (MusicShuffleDeck *deck)
{
	GList *node;
	gpointer *items;
	guint length;
	guint i;

	g_return_if_fail(deck != NULL);

	length = g_list_length(deck->ids);
	if (length < 2)
		return;

	items = g_new(gpointer, length);
	for (node = deck->ids, i = 0; node != NULL; node = node->next, i++)
		items[i] = node->data;

	for (i = length - 1; i > 0; i--)
	{
		guint other = g_random_int_range(0, i + 1);
		gpointer value = items[i];

		items[i] = items[other];
		items[other] = value;
	}

	for (node = deck->ids, i = 0; node != NULL; node = node->next, i++)
		node->data = items[i];

	g_free(items);
}

gboolean
music_shuffle_deck_take_next (MusicShuffleDeck *deck,
                              guint *id)
{
	GList *first;

	g_return_val_if_fail(deck != NULL, FALSE);
	g_return_val_if_fail(id != NULL, FALSE);

	first = deck->ids;
	if (first == NULL)
		return FALSE;

	*id = GPOINTER_TO_UINT(first->data);
	deck->ids = g_list_delete_link(deck->ids, first);
	return TRUE;
}

guint
music_shuffle_deck_length (MusicShuffleDeck *deck)
{
	g_return_val_if_fail(deck != NULL, 0);

	return g_list_length(deck->ids);
}
