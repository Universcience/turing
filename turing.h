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

#ifndef TURING_H_
#define TURING_H_

#include <stdbool.h>

typedef enum {
	LEFT  = -1,
	RIGHT = +1
} direction;

typedef char symbol;  // Printable symbol
typedef unsigned sid; // Symbol ID in alphabet

typedef int state;
#define HALT ((state) -1)

typedef struct {
	sid sym;       // Printed symbol
	direction dir; // Head shift
	state ns;      // New state
} step;

// Backstep = { Erased symbol, Head return, Former state }
typedef step bstep;

typedef struct {
	sid* contents; // Contents of the tape
	unsigned size; // Total physical size
	unsigned head; // Current head offset
	symbol blank;  // Default tape filling
} tape;

typedef struct {
	symbol* alph; // Display alphabet
	unsigned as;  // Alphabet size
	unsigned ns;  // Number of states
	step** tf;    // Transition function
	state s;      // Current state
	tape t;       // Machine tape
} machine;

typedef struct {
	bstep* hist;   // History of all backwards steps
	unsigned size; // Total length of history
	unsigned n;    // Current position
} history;

// Create a new TM on a blank (alph[0]) tape with the provided characteristics.
machine* new_machine (symbol* alph, unsigned as, unsigned ns, step** instructions, state start);

// Save/load a machine in a text file (with or without tape and current state).
machine* load_machine (const char* filename, bool snapshot);
void save_machine (const char* filename, machine* m, bool snapshot);

// Step forwards and backwards.
void do_step (machine* m, history* h); // h = NULL if you don't care
void do_bstep (machine* m, history* h);

/* TM file format

Bracketed elements are required iff is snapshot is true.
Excess characters at end of file are ignored and may be used for comments.
The first symbol of the alphabet will serve as tape filling (blank symbol).

TMFILE -> NS '\n' AS '\n' ALPH '\n' TF [S '\n' TS '\n' HEAD '\n' TAPE]

NS -> A positive integer (Number of states)
AS -> A positive integer (Alphabet size)
ALPH -> A string of AS chars (Alphabet)

TF -> ST^NS (Transition function)
ST -> STEP^AS '\n' (State transition)
STEP -> SYM DIR NS ' ' (Single transition)
SYM -> An integer in [0;AS[ (Printed symbol)
DIR -> '<' for LEFT ; '>' for RIGHT (Direction)
NS -> An integer in [0;NS[ (New state)

S -> An integer in [0;NS[ (Current state)
TS -> A positive integer (Tape size)
HEAD -> An integer in [0;TS[ (Head offset)
TAPE -> A string of TS chars found in ALPH (Tape contents)
*/

#endif // TURING_H_
