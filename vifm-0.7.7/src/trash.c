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

#include "trash.h"

#ifdef _WIN32
#include <dirent.h> /* DIR */
#endif

#include <sys/stat.h> /* stat */
#include <unistd.h> /* lstat */

#include <assert.h> /* assert() */
#include <errno.h> /* errno */
#include <stddef.h> /* NULL size_t */
#include <stdio.h> /* snprintf() */
#include <stdlib.h> /* free() */
#include <string.h> /* strchr() strcmp() strdup() strlen() strspn() */

#include "cfg/config.h"
#include "menus/menus.h"
#include "utils/fs.h"
#include "utils/fs_limits.h"
#include "utils/log.h"
#include "utils/mntent.h"
#include "utils/path.h"
#include "utils/str.h"
#include "utils/string_array.h"
#include "utils/utils.h"
#include "background.h"
#include "ops.h"
#include "registers.h"
#include "undo.h"

#define ROOTED_SPEC_PREFIX "%r/"
#define ROOTED_SPEC_PREFIX_LEN (sizeof(ROOTED_SPEC_PREFIX) - 1)

/* Describes file location relative to one of registered trash directories.
 * Argument for get_resident_type_traverser().*/
typedef enum
{
	TRT_OUT_OF_TRASH,         /* Not in trash. */
	TRT_IN_TRASH,             /* Direct child of one of trash directories. */
	TRT_IN_REMOVED_DIRECTORY, /* Child of one of removed directories. */
}
TrashResidentType;

/* Client of the traverse_specs() function.  Should return non-zero to stop
 * traversal. */
typedef int (*traverser)(const char base_dir[], const char trash_dir[],
		void *arg);

/* List of trash directories. */
typedef struct
{
	char **trashes; /* List of trashes. */
	int ntrashes;   /* Number of elements in trashes array. */
}
trashes_list;

/* State for get_list_of_trashes() traverser. */
typedef struct
{
	trashes_list *list; /* List of valid trash directories. */
	const char *spec;   /* Trash directory name specification. */
}
get_list_of_trashes_traverser_state;

static int validate_spec(const char spec[]);
static int create_trash_dir(const char trash_dir[]);
static void empty_trash_dirs(void);
static void empty_trash_dir(const char trash_dir[]);
static void empty_trash_list(void);
static trashes_list get_list_of_trashes(void);
static int get_list_of_trashes_traverser(struct mntent *entry, void *arg);
static int is_trash_valid(const char trash_dir[]);
static char * pick_trash_dir(const char base_dir[]);
static int pick_trash_dir_traverser(const char base_dir[],
		const char trash_dir[], void *arg);
static int is_rooted_trash_dir(const char spec[]);
static TrashResidentType get_resident_type(const char path[]);
static int get_resident_type_traverser(const char path[],
		const char trash_dir[], void *arg);
static int is_trash_directory_traverser(const char path[],
		const char trash_dir[], void *arg);
static void traverse_specs(const char base_dir[], traverser client, void *arg);
static char * get_rooted_trash_dir(const char base_dir[], const char spec[]);
static char * format_root_spec(const char spec[], const char mount_point[]);

static char **specs;
static int nspecs;

int
set_trash_dir(const char new_specs[])
{
	char **dirs = NULL;
	int ndirs = 0;

	int error;
	char *free_this;
	char *spec;

	error = 0;
	free_this = strdup(new_specs);
	spec = free_this;

	for(;;)
	{
		char *const p = until_first(spec, ',');
		const int last_element = *p == '\0';
		*p = '\0';

		if(!validate_spec(spec))
		{
			error = 1;
			break;
		}

		ndirs = add_to_string_array(&dirs, ndirs, 1, spec);

		if(last_element)
		{
			break;
		}
		spec = p + 1;
	}

	free(free_this);

	if(!error)
	{
		free_string_array(specs, nspecs);
		specs = dirs;
		nspecs = ndirs;

		copy_str(cfg.trash_dir, sizeof(cfg.trash_dir), new_specs);
	}
	else
	{
		free_string_array(dirs, ndirs);
	}

	return error;
}

/* Validates trash directory specification.  Returns non-zero if it's OK,
 * otherwise zero is returned and an error message is displayed. */
static int
validate_spec(const char spec[])
{
	if(is_path_absolute(spec))
	{
		if(create_trash_dir(spec) != 0)
		{
			return 0;
		}
	}
	else if(!is_rooted_trash_dir(spec))
	{
		show_error_msgf("Error Setting Trash Directory",
				"The path specification is of incorrect format: %s", spec);
		return 0;
	}
	return 1;
}

/* Ensures existence of trash directory.  Displays error message dialog, if
 * directory creation failed.  Returns zero on success, otherwise non-zero value
 * is returned. */
static int
create_trash_dir(const char trash_dir[])
{
	LOG_FUNC_ENTER;

	if(try_create_trash_dir(trash_dir) != 0)
	{
		show_error_msgf("Error Setting Trash Directory",
				"Could not set trash directory to %s: %s", trash_dir, strerror(errno));
		return 1;
	}

	return 0;
}

int
try_create_trash_dir(const char trash_dir[])
{
	LOG_FUNC_ENTER;

	if(is_dir_writable(trash_dir))
	{
		return 0;
	}

	return make_dir(trash_dir, 0777);
}

void
empty_trash(void)
{
	clean_regs_with_trash();
	empty_trash_dirs();
	clean_cmds_with_trash();
	empty_trash_list();
}

/* Empties all trash directories (all specifications on all mount points are
 * expanded). */
static void
empty_trash_dirs(void)
{
	const trashes_list list = get_list_of_trashes();
	int i;
	for(i = 0; i < list.ntrashes; i++)
	{
		empty_trash_dir(list.trashes[i]);
	}

	free_string_array(list.trashes, list.ntrashes);
}

/* Removes all files inside given trash directory (even those that this instance
 * of vifm is not aware of). */
static void
empty_trash_dir(const char trash_dir[])
{
#ifndef _WIN32
	char cmd[25 + strlen(trash_dir)*2 + 1];
	char *escaped;

	escaped = escape_filename(trash_dir, 0);
	snprintf(cmd, sizeof(cmd), "sh -c 'rm -rf %s/* %s/.[!.]*'", escaped, escaped);
	free(escaped);

	start_background_job(cmd, 0);
#else
	DIR *dir;
	struct dirent *d;

	dir = opendir(trash_dir);
	if(dir == NULL)
		return;
	while((d = readdir(dir)) != NULL)
	{
		if(!is_builtin_dir(d->d_name))
		{
			char full[PATH_MAX];
			snprintf(full, sizeof(full), "%s/%s", trash_dir, d->d_name);
			perform_operation(OP_REMOVESL, NULL, full, NULL);
		}
	}
	closedir(dir);
#endif
}

static void
empty_trash_list(void)
{
	int i;

	for(i = 0; i < nentries; i++)
	{
		free(trash_list[i].path);
		free(trash_list[i].trash_name);
	}
	free(trash_list);
	trash_list = NULL;
	nentries = 0;
}

int
add_to_trash(const char path[], const char trash_name[])
{
	void *p;

	if(!exists_in_trash(trash_name))
	{
		return -1;
	}

	if(is_in_trash(trash_name))
	{
		return 0;
	}

	if((p = realloc(trash_list, sizeof(*trash_list)*(nentries + 1))) == NULL)
	{
		return -1;
	}
	trash_list = p;

	trash_list[nentries].path = strdup(path);
	trash_list[nentries].trash_name = strdup(trash_name);
	if(trash_list[nentries].path == NULL ||
			trash_list[nentries].trash_name == NULL)
	{
		free(trash_list[nentries].path);
		free(trash_list[nentries].trash_name);
		return -1;
	}

	nentries++;
	return 0;
}

int
is_in_trash(const char trash_name[])
{
	int i;
	for(i = 0; i < nentries; i++)
	{
		if(stroscmp(trash_list[i].trash_name, trash_name) == 0)
			return 1;
	}
	return 0;
}

char **
list_trashes(int *ntrashes)
{
	trashes_list list = get_list_of_trashes();
	*ntrashes = list.ntrashes;
	return list.trashes;
}

/* Lists all non-empty trash directories.  Caller should free array and all its
 * elements using free(). */
static trashes_list
get_list_of_trashes(void)
{
	trashes_list list =
	{
		.trashes = NULL,
		.ntrashes = 0,
	};

	int i;
	for(i = 0; i < nspecs; i++)
	{
		const char *const spec = specs[i];
		if(is_rooted_trash_dir(spec))
		{
			get_list_of_trashes_traverser_state state =
			{
				.list = &list,
				.spec = spec,
			};
			(void)traverse_mount_points(&get_list_of_trashes_traverser, &state);
		}
		else if(is_trash_valid(spec))
		{
			list.ntrashes = add_to_string_array(&list.trashes, list.ntrashes, 1,
					spec);
		}
	}

	return list;
}

/* traverse_mount_points() client that collects valid trash directories into a
 * list the base_dir. */
static int
get_list_of_trashes_traverser(struct mntent *entry, void *arg)
{
	get_list_of_trashes_traverser_state *const params = arg;
	trashes_list *const list = params->list;
	const char *const spec = params->spec;

	char *const trash_dir = format_root_spec(spec, entry->mnt_dir);
	if(is_trash_valid(trash_dir))
	{
		list->ntrashes = add_to_string_array(&list->trashes, list->ntrashes, 1,
				trash_dir);
	}
	free(trash_dir);

	return 0;
}

/* Checks whether trash directory is valid (e.g. exists, writable and
 * non-empty).  Returns non-zero if so, otherwise zero is returned. */
static int
is_trash_valid(const char trash_dir[])
{
	return is_dir_writable(trash_dir) && !is_dir_empty(trash_dir);
}

int
exists_in_trash(const char trash_name[])
{
	return path_exists(trash_name);
}

int
restore_from_trash(const char trash_name[])
{
	int i;
	char full[PATH_MAX];
	char buf[PATH_MAX];

	for(i = 0; i < nentries; i++)
	{
		if(stroscmp(trash_list[i].trash_name, trash_name) == 0)
			break;
	}
	if(i >= nentries)
		return -1;
	
	copy_str(buf, sizeof(buf), trash_list[i].path);
	copy_str(full, sizeof(full), trash_list[i].trash_name);
	if(perform_operation(OP_MOVE, NULL, full, trash_list[i].path) == 0)
	{
		char *msg, *p;
		size_t len;

		cmd_group_continue();

		msg = replace_group_msg(NULL);
		len = strlen(msg);
		p = realloc(msg, COMMAND_GROUP_INFO_LEN);
		if(p == NULL)
			len = COMMAND_GROUP_INFO_LEN;
		else
			msg = p;

		snprintf(msg + len, COMMAND_GROUP_INFO_LEN - len, "%s%s",
				(msg[len - 2] != ':') ? ", " : "", strchr(trash_name, '_') + 1);
		replace_group_msg(msg);
		free(msg);

		add_operation(OP_MOVE, NULL, NULL, full, buf);
		cmd_group_end();
		remove_from_trash(trash_name);
		return 0;
	}
	return -1;
}

int
remove_from_trash(const char trash_name[])
{
	int i;
	for(i = 0; i < nentries; i++)
	{
		if(stroscmp(trash_list[i].trash_name, trash_name) == 0)
			break;
	}
	if(i >= nentries)
		return -1;

	free(trash_list[i].path);
	free(trash_list[i].trash_name);
	memmove(trash_list + i, trash_list + i + 1,
			sizeof(*trash_list)*((nentries - 1) - i));

	nentries--;
	return 0;
}

char *
gen_trash_name(const char base_dir[], const char name[])
{
	struct stat st;
	char buf[PATH_MAX];
	int i;
	char *const trash_dir = pick_trash_dir(base_dir);

	if(trash_dir == NULL)
	{
		return NULL;
	}

	i = 0;
	do
	{
		snprintf(buf, sizeof(buf), "%s/%03d_%s", trash_dir, i++, name);
		chosp(buf);
	}
	while(lstat(buf, &st) == 0);

	free(trash_dir);

	return strdup(buf);
}

/* Picks trash directory basing on original directory of a file that is being
 * trashed.  Returns absolute path to picked trash directory on success,
 * otherwise NULL is returned. */
static char *
pick_trash_dir(const char base_dir[])
{
	char *trash_dir = NULL;
	traverse_specs(base_dir, &pick_trash_dir_traverser, &trash_dir);
	return trash_dir;
}

/* traverse_specs client that finds first available trash directory suitable for
 * the base_dir. */
static int
pick_trash_dir_traverser(const char base_dir[], const char trash_dir[],
		void *arg)
{
	if(try_create_trash_dir(trash_dir) == 0)
	{
		char **const result = arg;
		*result = strdup(trash_dir);
		return 1;
	}
	return 0;
}

/* Checks whether the spec refers to a rooted trash directory.  Returns non-zero
 * if so, otherwise non-zero is returned. */
static int
is_rooted_trash_dir(const char spec[])
{
	return starts_with_lit(spec, ROOTED_SPEC_PREFIX)
	    && spec[ROOTED_SPEC_PREFIX_LEN] != '\0';
}

int
is_under_trash(const char path[])
{
	return get_resident_type(path) != TRT_OUT_OF_TRASH;
}

/* Gets status of file relative to trash directories.  Returns the status. */
static TrashResidentType
get_resident_type(const char path[])
{
	TrashResidentType resident_type = TRT_OUT_OF_TRASH;
	traverse_specs(path, &get_resident_type_traverser, &resident_type);
	return resident_type;
}

/* traverse_specs client that check that the path is under one of trash
 * directories.  Accepts pointer to TrashResidentType as argument. */
static int
get_resident_type_traverser(const char path[], const char trash_dir[],
		void *arg)
{
	if(path_starts_with(path, trash_dir))
	{
		TrashResidentType *const result = arg;
		char *const parent_dir = strdup(path);

		remove_last_path_component(parent_dir);
		*result = paths_are_equal(parent_dir, trash_dir)
		        ? TRT_IN_TRASH
		        : TRT_IN_REMOVED_DIRECTORY;

		free(parent_dir);

		return 1;
	}
	return 0;
}

int
is_trash_directory(const char path[])
{
	int trash_directory = 0;
	traverse_specs(path, &is_trash_directory_traverser, &trash_directory);
	return trash_directory;
}

/* traverse_specs client that check that the path is one of trash
 * directories. */
static int
is_trash_directory_traverser(const char path[], const char trash_dir[],
		void *arg)
{
	if(stroscmp(path, trash_dir) == 0)
	{
		int *const result = arg;
		*result = 1;
		return 1;
	}
	return 0;
}

/* Calls client traverser for each trash directory specification defined by
 * specs array. */
static void
traverse_specs(const char base_dir[], traverser client, void *arg)
{
	int i;
	for(i = 0; i < nspecs; i++)
	{
		char *to_free = NULL;
		const char *trash_dir;

		const char *const spec = specs[i];
		if(is_rooted_trash_dir(spec))
		{
			to_free = get_rooted_trash_dir(base_dir, spec);
			trash_dir = to_free;
		}
		else
		{
			trash_dir = spec;
		}

		if(trash_dir != NULL && client(base_dir, trash_dir, arg))
		{
			free(to_free);
			break;
		}

		free(to_free);
	}
}

/* Expands rooted trash directory specification into a string.  Returns NULL on
 * error, otherwise newly allocated string that should be freed by the caller is
 * returned. */
static char *
get_rooted_trash_dir(const char base_dir[], const char spec[])
{
	char full[PATH_MAX];
	if(get_mount_point(base_dir, sizeof(full), full) == 0)
	{
		return format_root_spec(spec, full);
	}

	return NULL;
}

/* Expands rooted trash directory specification into a string.  Returns newly
 * allocated string that should be freed by the caller. */
static char *
format_root_spec(const char spec[], const char mount_point[])
{
	return format_str("%s/%s", mount_point, spec + ROOTED_SPEC_PREFIX_LEN);
}

const char *
get_real_name_from_trash_name(const char trash_path[])
{
	const char *real_name = after_last(trash_path, '/');

	assert(is_path_absolute(trash_path) && "Expected full path to a file.");

	if(get_resident_type(trash_path) == TRT_IN_TRASH)
	{
		const size_t prefix_len = strspn(real_name, "0123456789");
		if(real_name[prefix_len] == '_')
		{
			real_name += prefix_len + 1;
		}
	}

	return real_name;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
