/* vifm
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

#ifndef VIFM__TRASH_H__
#define VIFM__TRASH_H__

typedef struct
{
	char *path;
	char *trash_name;
}
trash_entry_t;

trash_entry_t *trash_list;
int nentries;

/* Parses trash directory name specification.  Sets value of cfg.trash_dir as a
 * side effect.  Returns non-zero in case of error, otherwise zero is
 * returned. */
int set_trash_dir(const char trash_dir[]);

/* Tries to create trash directory.  Returns zero on success, otherwise non-zero
 * value is returned. */
int try_create_trash_dir(const char trash_dir[]);

void empty_trash(void);

int add_to_trash(const char path[], const char trash_name[]);

int is_in_trash(const char trash_name[]);

/* Lists all non-empty trash directories.  Puts number of elements to *ntrashes.
 * Caller should free array and all its elements using free().  On error returns
 * NULL and sets *ntrashes to zero. */
char ** list_trashes(int *ntrashes);

/* Checks whether file with given name exists in the trash directory.  Returns
 * non-zero if so, otherwise zero is returned. */
int exists_in_trash(const char trash_name[]);

/* Restores a file specified by its trash_name (from trash_list array).  Returns
 * zero on success, otherwise non-zero is returned. */
int restore_from_trash(const char trash_name[]);

int remove_from_trash(const char trash_name[]);

/* Generates unique name for a file at base_dir/name in a trash directory.
 * Returns string containing full path that needs to be freed by caller, if no
 * trash directory available NULL is returned. */
char * gen_trash_name(const char base_dir[], const char name[]);

/* Checks whether given absolute path points to a file under trash directory.
 * Returns non-zero if so, otherwise zero is returned. */
int is_under_trash(const char path[]);

/* Checks whether given absolute path points to a trash directory.  Returns
 * non-zero if so, otherwise zero is returned. */
int is_trash_directory(const char path[]);

/* Gets pointer to real name part of the trash path (which must be absolute).
 * Returns that pointer. */
const char * get_real_name_from_trash_name(const char trash_path[]);

#endif /* VIFM__TRASH_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
