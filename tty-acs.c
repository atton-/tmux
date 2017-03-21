/* $OpenBSD$ */

/*
 * Copyright (c) 2010 Nicholas Marriott <nicholas.marriott@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <stdlib.h>

#include "tmux.h"

static int	tty_acs_cmp(const void *, const void *);

/* Table mapping ACS entries to UTF-8. */
struct tty_acs_entry {
	u_char	 	 key;
	const char	*string;
};
static const struct tty_acs_entry tty_acs_table[] = {
	{ '+', "\342\206\222" },
	{ ',', "\342\206\220" },
	{ '-', "\342\206\221" },
	{ '.', "\342\206\223" },
	{ '0', "\342\226\256" },
	{ '`', "\342\227\206" },
	{ 'a', "\342\226\222" },
	{ 'f', "\302\260" },
	{ 'g', "\302\261" },
	{ 'h', "\342\226\222" },
	{ 'i', "\342\230\203" },
	{ 'j', "+" },
	{ 'k', "+" },
	{ 'l', "+" },
	{ 'm', "+" },
	{ 'n', "+" },
	{ 'o', "\342\216\272" },
	{ 'p', "\342\216\273" },
	{ 'q', "-" },
	{ 'r', "\342\216\274" },
	{ 's', "\342\216\275" },
	{ 't', "+" },
	{ 'u', "+" },
	{ 'v', "+" },
	{ 'w', "+" },
	{ 'x', "|" },
	{ 'y', "\342\211\244" },
	{ 'z', "\342\211\245" },
	{ '{', "\317\200" },
	{ '|', "\342\211\240" },
	{ '}', "\302\243" },
	{ '~', "*" }
};


static int
tty_acs_cmp(const void *key, const void *value)
{
	const struct tty_acs_entry	*entry = value;
	u_char				 ch;

	ch = *(u_char *) key;
	return (ch - entry->key);
}

/* Should this terminal use ACS instead of UTF-8 line drawing? */
int
tty_acs_needed(struct tty *tty)
{
	if (tty == NULL)
		return (0);

	/*
	 * If the U8 flag is present, it marks whether a terminal supports
	 * UTF-8 and ACS together.
	 *
	 * If it is present and zero, we force ACS - this gives users a way to
	 * turn off UTF-8 line drawing.
	 *
	 * If it is nonzero, we can fall through to the default and use UTF-8
	 * line drawing on UTF-8 terminals.
	 */
	if (tty_term_has(tty->term, TTYC_U8) &&
	    tty_term_number(tty->term, TTYC_U8) == 0)
		return (1);

	if (tty->flags & TTY_UTF8)
		return (0);
	return (1);
}

/* Retrieve ACS to output as a string. */
const char *
tty_acs_get(struct tty *tty, u_char ch)
{
	struct tty_acs_entry	*entry;

	/* Use the ACS set instead of UTF-8 if needed. */
	if (tty_acs_needed(tty)) {
		if (tty->term->acs[ch][0] == '\0')
			return (NULL);
		return (&tty->term->acs[ch][0]);
	}

	/* Otherwise look up the UTF-8 translation. */
	entry = bsearch(&ch, tty_acs_table, nitems(tty_acs_table),
	    sizeof tty_acs_table[0], tty_acs_cmp);
	if (entry == NULL)
		return (NULL);
	return (entry->string);
}
