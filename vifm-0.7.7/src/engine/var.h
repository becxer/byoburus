/* vifm
 * Copyright (C) 2012 xaizek.
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

#ifndef VIFM__ENGINE__VAR_H__
#define VIFM__ENGINE__VAR_H__

/* This unit provides all necessary to work with variables. */


/* Enumeration of variable types. */
typedef enum
{
	VTYPE_STRING, /* Regular string value. */
	VTYPE_INT, /* Integer, which is also used for boolean. */
	VTYPE_ERROR, /* Signals about fail. */
}VarType;

/* Union of possible variable contents. */
typedef union
{
	char *string; /* String value for VTYPE_STRING, should be copied to use it. */
	int integer; /* VTYPE_INT value. */

	const char *const_string; /* Convenience field handy to create variables. */
}var_val_t;

/* Structure for script variable. */
typedef struct
{
	VarType type; /* Variable type. */
	var_val_t value; /* Value depending on type. */
}var_t;


/* Returns variable, which evaluates to false. */
var_t var_false(void);

/* Returns variable, which signals about failed operation. */
var_t var_error(void);

/* Constructs variable in convenient way. */
var_t var_new(VarType type, const var_val_t value);

/* Convenient function to clones a variable. */
var_t var_clone(var_t var);

/* Converts variable to a string.  Returns new string, which should be freed by
 * the caller. */
char * var_to_string(const var_t var);

/* Converts variable to an integer.  Returns integer value, parsing of a string
 * is performed to get integer value. */
int var_to_integer(const var_t var);

/* Converts variable to a boolean.  Returns non-zero if value is evaluated to
 * true. */
int var_to_boolean(const var_t var);

/* Frees resources allocated for the var if any. */
void var_free(const var_t var);

#endif /* VIFM__ENGINE__VAR_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
