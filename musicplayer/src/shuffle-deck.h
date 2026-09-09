#ifndef MUSIC_SHUFFLE_DECK_H
#define MUSIC_SHUFFLE_DECK_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _MusicShuffleDeck MusicShuffleDeck;

MusicShuffleDeck *music_shuffle_deck_new (void);
void music_shuffle_deck_free (MusicShuffleDeck *deck);
void music_shuffle_deck_clear (MusicShuffleDeck *deck);
void music_shuffle_deck_rebuild (MusicShuffleDeck *deck,
                                 const guint *ids,
                                 gsize count,
                                 guint excluded_id);
void music_shuffle_deck_add (MusicShuffleDeck *deck, guint id);
void music_shuffle_deck_remove (MusicShuffleDeck *deck, guint id);
void music_shuffle_deck_shuffle (MusicShuffleDeck *deck);
gboolean music_shuffle_deck_take_next (MusicShuffleDeck *deck, guint *id);
guint music_shuffle_deck_length (MusicShuffleDeck *deck);

G_END_DECLS

#endif /* MUSIC_SHUFFLE_DECK_H */
