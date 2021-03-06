/*  Quick and dirty implementation of ncurses-based Turing machines.
 *  Copyright (C) 2017-2019 - Jérôme Kirman
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "turing.h"

void expand (tape* t);

machine* new_machine (symbol* alph, size_t as, size_t ns, step** instructions, state start)
{
	size_t ts = 42; // Must be even.
	sid* tc = malloc(ts * sizeof(sid));
	for (size_t i = 0 ; i < ts ; i++)
		tc[i] = 0;
	tape t = {tc, ts, ts/2, 0};

	machine* m = malloc(sizeof(*m));
	m->alph = alph;
	m->as = as;
	m->ns = ns;
	m->tf = instructions;
	m->s = start;
	m->t = t;

	return m;
}

//FIXME: Add sanity tests on contents.
//TODO: Add snapshots by parsing machine state.
machine* load_machine (const char* filename, bool snapshot)
{
	// Read file contents
	int fd = open(filename, O_RDONLY);

	off_t len = lseek(fd, 0, SEEK_END),
				nol = lseek(fd, 0, SEEK_SET);
	char* contents = calloc((size_t) len, sizeof(char));
	off_t rs = read(fd, contents, (size_t) len);

	close(fd);
	if (fd == -1 || len == -1 || nol == -1
	|| !contents || rs != len || snapshot)
	{
		free(contents);
		return NULL;
	}

	// Parse machine specs
	char* head = contents;
	size_t ns = strtoul(head, &head, 10);
	size_t as = strtoul(head, &head, 10);
	symbol* alph = malloc(as * sizeof(symbol));
	memcpy(alph, ++head, as);
	head += as;
	
	// Parse transition function
	step** tf = malloc(ns * sizeof(step*));
	for (size_t i = 0 ; i < ns ; i++) {
		tf[i] = malloc(as * sizeof(step));
		for (size_t j = 0 ; j < as ; j++) {
			step s;
			s.sym = strtoul(head, &head, 10);
			s.dir = ( *head == '<' ) ? LEFT : RIGHT;
			s.ns = (state) strtol(++head, &head, 10);
			tf[i][j] = s;
		}
	}

	free(contents);

	return new_machine(alph, as, ns, tf, 0);
}

/* Tape size expansion (x2) :
 *
 *      Head
 *      v
 * [A|B|C|D|E|F]             < Old tape
 *   \ \ \ \ \ \
 *    \ \ \ \ \ \
 *     \ \ \ \ \ \
 *      \ \ \ \ \ \
 *       \ \ \ \ \ \
 * [0|0|0|A|B|C|D|E|F|0|0|0] < New tape
 *            ^
 *            Head
 *
 * NOTE: Original tape size MUST be even,
 * or data in the last cell will be lost.
 */
void expand (tape* t)
{
	t->size *= 2;
	assert (t->size < INT_MAX);

	sid* ntape = malloc(t->size * sizeof(sid));

	size_t shift = t->size/4;
	for (size_t i = 0 ; i < t->size ; ++i)
		if (i >= shift && i < 3*shift)
			ntape[i] = t->contents[i-shift];
		else
			ntape[i] = t->blank;
	free(t->contents);
	t->contents = ntape;
	t->head += shift;
}

void do_step (machine* m, history* h)
{
	if (m->s == HALT) { return; }

	tape* t = &m->t;
	step next = m->tf[m->s][t->contents[t->head]];

	// History logging
	if (h) {
		bstep back = {t->contents[t->head], -next.dir, m->s};
		h->hist[h->n++] = back;

		if (h->n == h->size)
			h->hist = realloc(h->hist, (h->size *= 2) * sizeof(bstep));
	}

	t->contents[t->head] = next.sym;
	t->head += (size_t) next.dir;
	m->s = next.ns;

	// Underflow expected on left move.
	if (t->head >= t->size)
		expand(t);
}

void do_bstep (machine* m, history* h)
{
	if (!h->n) { return; }

	tape* t = &m->t;
	bstep back = h->hist[--(h->n)];

	m->s = back.ns;
	t->head += (size_t) back.dir;
	t->contents[t->head] = back.sym;
}
