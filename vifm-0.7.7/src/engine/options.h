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

#ifndef VIFM__ENGINE__OPTIONS_H__
#define VIFM__ENGINE__OPTIONS_H__

/* The module processes :set command arguments and handles all operations on
 * options. */

/* Type of an option. */
typedef enum
{
	OPT_BOOL,    /* A boolean. */
	OPT_INT,     /* An integer. */
	OPT_STR,     /* A string of undefined format. */
	OPT_STRLIST, /* A comma separated list of strings. */
	OPT_ENUM,    /* An enumeration (one value at a time). */
	OPT_SET,     /* A set (multiple or none values at a time). */
	OPT_CHARSET, /* A set of single characters (multiple or none values at a
	                time). */
}OPT_TYPE;

/* Operation on an option. */
typedef enum
{
	OP_ON,       /* Boolean value was turned on. */
	OP_OFF,      /* Boolean value was turned off. */
	OP_SET,      /* Value set. */
	OP_MODIFIED, /* Value added/removed (for OPT_INT,  OPT_SET and OPT_STR). */
	OP_RESET,    /* Value reseted to default. */
}OPT_OP;

typedef union
{
	int bool_val;  /* Value of OPT_BOOL type. */
	int int_val;   /* Value of OPT_INT type. */
	char *str_val; /* Value of OPT_STR or OPT_STRLIST types. */
	int enum_item; /* Value of OPT_ENUM type. */
	int set_items; /* Value of OPT_SET type. */
}optval_t;

/* Function type for option handler. */
typedef void (*opt_handler)(OPT_OP op, optval_t val);

/* Initializes option module.  opts_changed_flag will be set to true after at
 * least one option will change it value using set_options(...) function. */
void init_options(int *opts_changed_flag);
/* Resets an option to its default value. */
void reset_option_to_default(const char name[]);
/* Resets all options to their default values. */
void reset_options_to_default(void);
/* Removes all options. */
void clear_options(void);
/* Adds an option. */
void add_option(const char name[], const char abbr[], OPT_TYPE type,
		int val_count, const char *vals[], opt_handler handler, optval_t def);
/* Sets option value without calling its change handler. */
void set_option(const char name[], optval_t val);
/* Processes :set command arguments.  Returns non-zero on error. */
int set_options(const char args[]);

/* Completes set arguments list. */
void complete_options(const char args[], const char **start);

/* Completes names of real options (no pseudo options like "all"). */
void complete_real_option_names(const char beginning[]);

#endif /* VIFM__ENGINE__OPTIONS_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
