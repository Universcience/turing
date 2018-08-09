/*  Quick and dirty implementation of ncurses-based Turing machines.
 *  Copyright (C) 2017-2018 - Jérôme Kirman
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

#include <curses.h>
#include <unistd.h>
#include <stdlib.h>

#include "turing.h"

//FIXME: Decouple simulation and interface steps.
//TODO: Tape GPS.

/* Keyboard commands :
 *
 * UP    - Speed up
 * DOWN  - Speed down
 * LEFT  - Play backwards
 * RIGHT - Play forward
 * SPACE - Pause
 * p - Also pause
 * n - Next step
 * b - Step back
 *
 * < - Pan left
 * > - Pan right
 * w - Follow head
 * x - Lock head
 * c - Center view
 *
 * h - Help (TODO)
 * q - Quit
 */

unsigned clamp (unsigned min, unsigned val, unsigned max)
{
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

/*    Instructions HUD blueprint
 *
 *   01230123456701234567012345670
 *  0+---+-------+-------+-------+
 *  1| S |   1   |   2   |   3   |
 *  2+---+-------+-------+-------+
 *  3| _ | # > 2 | _ > 3 | # < 3 |
 *  4|   |       |       |       |
 *  5| # | # > H | # > 2 | # < 1 |
 *  6+---+-------+-------+-------+
 */

void flatline (unsigned n, bool fill)
{
	addstr(fill ? "+---" : "|   ");
	for (unsigned i = 0 ; i < n ; ++i)
		addstr(fill ? "+-------" : "|       " );
	addstr(fill ? "+" : "|");
}

void print_instructions (machine* m, unsigned x, unsigned y)
{
	move(x,y);
	flatline(m->ns, true);

	move(y+1, x);
	addstr("| S ");
	for (unsigned i = 0 ; i < m->ns ; ++i)
		printw("|   %i%c  ", i, (int) i == m->s ? '*' : ' '); // FIXME: i>9
	addstr("|");

	move(y+2, x);
	flatline(m->ns, true);

	for (unsigned i = 0 ; i < m->as ; ++i) {
		move(y+3+2*i, x);
		printw("| %c ", m->alph[i]);
		for (unsigned j = 0 ; j < m->ns ; ++j) {
			printw("| %c %c ",
			  m->alph[m->tf[j][i].sym],
			  m->tf[j][i].dir == LEFT ? '<' : '>'
			);
			state s = m->tf[j][i].ns;
			if (s == HALT)
				printw("H ");
			else
				printw("%i ", s);
		}
		printw("|");

		move(y+4+2*i, x);
		flatline(m->ns, i == m->as-1);
	}
}

int main (int argc, char* argv[])
{
	char* bbfile = "assets/bb5c.tm";
	if (argc == 2)
		bbfile = argv[1];

	machine* bb = load_machine(bbfile, false);
	history bh;
	bh.hist = calloc(42, sizeof(bstep));
	bh.size = 42;
	bh.n = 0;

	// Init ncurses
	initscr(); cbreak(); noecho();
	keypad(stdscr, TRUE);
	timeout(0);

	char cmd;
	unsigned view = 0;
	unsigned speed = 16;
	bool pause = false;
	bool forward = true;
	bool follow = true;
	bool lock = false;
	bool over = false;

	while (!over)
	{
		// Display machine state
		move(0,0);
		for (int i = 0 ; i < COLS ; ++i)
			addch(' ');

		move(0, 5);
		if (pause)
			addstr("[PAUSED]");

		move(0, 15);
		printw("Step : %i", bh.n);

		move(0, 30);
		for (unsigned i = 0 ; i < 21-speed ; ++i)
			addch(forward ? '>' : '<');

		move(0, 52);
		if (lock)
			addstr(" [LOCK] ");
		else if (follow)
			addstr("[FOLLOW]");

		print_instructions(bb, 2, 2);

		// Center view if required
		if (lock || (follow && (bb->t.head < view || bb->t.head >= view + COLS)))
			view = bb->t.head - COLS/2;

		if (view > (unsigned) -COLS)
			view = 0;

		// Draw tape
		for (unsigned i = view ; i < view + COLS ; ++i)
			if (i < bb->t.size)
				mvaddch(LINES/2, i-view, bb->alph[bb->t.contents[i]]);
			else
				mvaddch(LINES/2, i-view, ' ');

		// Draw head, refresh and erase now-former head
		mvaddch(LINES/2 - 1, bb->t.head-view, 'V');
		move(0,0);
		cmd = getch();
		flushinp();
		mvaddch(LINES/2 - 1, bb->t.head-view, ' ');

		// Keyboard commands
		switch (cmd) {
			case 3: // Up
				speed--;
				break;
			case 2: // Down
				speed++;
				break;
			case 4: // Left
				forward = false;
				break;
			case 5: // Right
				forward = true;
				break;
			case 'n':
				pause = true;
				do_step(bb, &bh);
				break;
			case 'b':
				pause = true;
				do_bstep(bb, &bh);
				break;
			case ' ':
			case 'p':
				pause ^= true;
				break;
			case 'w':
				follow ^= true;
				break;
			case 'x':
				lock ^= true;
				break;
			case 'c':
				view = bb->t.head - COLS/2;
				break;
			case '<':
				view--;
				break;
			case '>':
				view++;
				break;
			case 'q':
				over = true;
				break;
			default:
				mvprintw(0, 0, "%i", cmd);
		}
		speed = clamp(1, speed, 20);

		// Beaver step
		usleep(1 << speed);

		if (pause)
			continue;

		if (forward)
			do_step(bb, &bh);
		else
			do_bstep(bb, &bh);
	}

	free(bh.hist);

	free(bb->t.contents);
	free(bb->alph);
	for (unsigned i = 0 ; i < bb->ns ; i++)
		free(bb->tf[i]);
	free(bb->tf);
	free(bb);

	endwin();
	return EXIT_SUCCESS;
}
