/* vifm
 * Copyright (C) 2013 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef VIFM__ESCAPE_H__
#define VIFM__ESCAPE_H__

#include <regex.h>

#include <curses.h>

#include "colors.h"
#include "utils/test_helpers.h"

/* Possible modes of processing escape codes. */
typedef enum
{
	ESM_SHORT,         /* "Normal" state for standard escape codes. */
	ESM_GOT_FG_PREFIX, /* After first number of xterm256 foreground sequence. */
	ESM_GOT_BG_PREFIX, /* After first number of xterm256 background sequence. */
	ESM_WAIT_FG_COLOR, /* After second number of xterm256 foreground sequence. */
	ESM_WAIT_BG_COLOR, /* After second number of xterm256 background sequence. */
}
EscStateMode;

/* Holds state of escape sequence parsing. */
typedef struct
{
	EscStateMode mode;   /* Current mode of processing escape codes. */

	int attrs;           /* Current set of attributes. */
	int fg;              /* Current foreground color. */
	int bg;              /* Current background color. */

	col_attr_t defaults; /* Default values of other fields. */
}
esc_state;

/* Returns a copy of the str with all escape sequences removed.  The string
 * returned should be freed by a caller. */
char * esc_remove(const char str[]);

/* Returns number of characters in the str taken by terminal escape
 * sequences. */
size_t esc_str_overhead(const char str[]);

/* Highlights all matches of the re regular expression in the line using color
 * inversion considering escape sequences in the line.  Returns processed string
 * that should be freed by the caller or NULL on error. */
char * esc_highlight_pattern(const char line[], const regex_t *re);

/* Prints at most whole line to a window with col and row initial offsets and
 * honoring maximum character positions specified by the max_width parameter.
 * Sets *printed to number of screen characters actually printed, it's set to
 * zero otherwise.  Returns offset in the line at which line processing was
 * stopped. */
int esc_print_line(const char line[], WINDOW *win, int col, int row,
		int max_width, int dry_run, esc_state *state, int *printed);

/* Initializes escape sequence parsing state with values from the defaults. */
void esc_state_init(esc_state *state, const col_attr_t *defaults);

TSTATIC_DEFS(
	const char * strchar2str(const char str[], int pos, size_t *screen_width);
)

#endif /* VIFM__ESCAPE_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
