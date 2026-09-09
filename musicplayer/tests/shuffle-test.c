#include <glib.h>

#include "shuffle-deck.h"

static void
test_excludes_current_and_visits_each_song (void)
{
	MusicShuffleDeck *deck = music_shuffle_deck_new();
	guint ids[] = { 10, 20, 30, 40, 50, 60 };
	gboolean seen[6] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
	guint id;
	guint count = 0;

	music_shuffle_deck_rebuild(deck, ids, G_N_ELEMENTS(ids), 30);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 5);

	while (music_shuffle_deck_take_next(deck, &id))
	{
		g_assert_cmpuint(id, !=, 30);
		for (guint i = 0; i < G_N_ELEMENTS(ids); i++)
		{
			if (ids[i] == id)
			{
				g_assert_false(seen[i]);
				seen[i] = TRUE;
				count++;
			}
		}
	}

	g_assert_cmpuint(count, ==, 5);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 0);
	music_shuffle_deck_free(deck);
}

static void
test_empty_and_single_song (void)
{
	MusicShuffleDeck *deck = music_shuffle_deck_new();
	guint id;
	guint only_id = 99;

	g_assert_false(music_shuffle_deck_take_next(deck, &id));
	music_shuffle_deck_rebuild(deck, &only_id, 1, only_id);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 0);
	g_assert_false(music_shuffle_deck_take_next(deck, &id));

	music_shuffle_deck_rebuild(deck, &only_id, 1, 0);
	g_assert_true(music_shuffle_deck_take_next(deck, &id));
	g_assert_cmpuint(id, ==, only_id);
	g_assert_false(music_shuffle_deck_take_next(deck, &id));
	music_shuffle_deck_free(deck);
}

static void
test_repeat_can_start_a_new_deck (void)
{
	MusicShuffleDeck *deck = music_shuffle_deck_new();
	guint id;
	guint count = 0;

	music_shuffle_deck_add(deck, 1);
	music_shuffle_deck_add(deck, 2);
	music_shuffle_deck_add(deck, 3);
	music_shuffle_deck_add(deck, 2);
	music_shuffle_deck_shuffle(deck);

	while (music_shuffle_deck_take_next(deck, &id))
		count++;
	g_assert_cmpuint(count, ==, 3);
	g_assert_false(music_shuffle_deck_take_next(deck, &id));

	music_shuffle_deck_clear(deck);
	music_shuffle_deck_add(deck, 1);
	music_shuffle_deck_add(deck, 2);
	music_shuffle_deck_add(deck, 3);
	music_shuffle_deck_shuffle(deck);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 3);

	music_shuffle_deck_free(deck);
}

static void
test_remove_and_add (void)
{
	MusicShuffleDeck *deck = music_shuffle_deck_new();
	guint id;

	music_shuffle_deck_add(deck, 7);
	music_shuffle_deck_add(deck, 8);
	music_shuffle_deck_remove(deck, 7);
	music_shuffle_deck_add(deck, 9);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 2);

	g_assert_true(music_shuffle_deck_take_next(deck, &id));
	g_assert_true(id == 8 || id == 9);
	g_assert_true(music_shuffle_deck_take_next(deck, &id));
	g_assert_true(id == 8 || id == 9);
	g_assert_cmpuint(music_shuffle_deck_length(deck), ==, 0);

	music_shuffle_deck_free(deck);
}

int
main (int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/shuffle/excludes-current-and-visits-each-song",
	                test_excludes_current_and_visits_each_song);
	g_test_add_func("/shuffle/repeat-can-start-new-deck",
	                test_repeat_can_start_a_new_deck);
	g_test_add_func("/shuffle/empty-and-single-song",
	                test_empty_and_single_song);
	g_test_add_func("/shuffle/remove-and-add",
	                test_remove_and_add);
	return g_test_run();
}
