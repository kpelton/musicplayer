#include <glib.h>

#include "music-side-queue.h"

static void
test_fifo_order_and_positions (void)
{
	MusicSideQueue *queue = music_side_queue_new();

	music_side_queue_enqueue(queue, 10);
	music_side_queue_enqueue(queue, 20);
	music_side_queue_enqueue(queue, 30);

	g_assert_true(music_side_queue_contains(queue, 10));
	g_assert_cmpuint(music_side_queue_get_position(queue, 10), ==, 1);
	g_assert_cmpuint(music_side_queue_get_position(queue, 30), ==, 3);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 10);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 20);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 30);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 0);

	g_object_unref(queue);
}

static void
test_remove_keeps_remaining_order (void)
{
	MusicSideQueue *queue = music_side_queue_new();

	music_side_queue_enqueue(queue, 1);
	music_side_queue_enqueue(queue, 2);
	music_side_queue_enqueue(queue, 3);
	music_side_queue_remove(queue, 2);

	g_assert_false(music_side_queue_contains(queue, 2));
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 1);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 3);
	g_assert_cmpuint(music_side_queue_dequeue(queue), ==, 0);

	g_object_unref(queue);
}

int
main (int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/side-queue/fifo-order-and-positions",
	                test_fifo_order_and_positions);
	g_test_add_func("/side-queue/remove-keeps-remaining-order",
	                test_remove_keeps_remaining_order);
	return g_test_run();
}
