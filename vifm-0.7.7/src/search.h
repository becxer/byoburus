/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2011 xaizek.
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

#ifndef VIFM__SEARCH_H__
#define VIFM__SEARCH_H__

#include "ui.h"

/* Search and navigation functions. */

/* The move argument specifies whether cursor in the view should be adjusted to
 * point to just found file in case of successful search.  Sets *found to
 * non-zero if pattern was found, otherwise it's assigned zero.  Returns
 * non-zero when a message was printed to a user, otherwise zero is returned. */
int find_pattern(FileView *view, const char pattern[], int backward, int move,
		int *const found);

/* Looks for a search match in specified direction from current cursor position
 * taking search wrapping into account.  Returns non-zero if something was
 * found, otherwise zero is returned. */
int goto_search_match(FileView *view, int backward);

/* Auxiliary functions. */

/* Prints results or error message about search operation to the user. */
void print_search_msg(const FileView *view, int backward);

/* Prints error message about failed search to the user. */
void print_search_fail_msg(const FileView *view, int backward);

#endif /* VIFM__SEARCH_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
