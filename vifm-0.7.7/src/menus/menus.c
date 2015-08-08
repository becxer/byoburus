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

#include "menus.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <curses.h>

#include <dirent.h> /* DIR */
#include <sys/stat.h>
#include <sys/types.h> /* pid_t */
#include <unistd.h> /* access() F_OK R_OK */

#include <assert.h> /* assert() */
#include <ctype.h> /* isspace() */
#include <errno.h> /* errno */
#include <stddef.h> /* NULL size_t */
#include <stdlib.h> /* free() malloc() */
#include <string.h> /* memmove() memset() strdup() strcat() strncat() strchr()
                       strlen() strrchr() */
#include <stdarg.h>
#include <signal.h>
#include <wchar.h> /* wchar_t wcscmp() */

#include "../cfg/config.h"
#include "../modes/cmdline.h"
#include "../modes/menu.h"
#include "../modes/modes.h"
#include "../utils/file_streams.h"
#include "../utils/fs.h"
#include "../utils/log.h"
#include "../utils/path.h"
#include "../utils/str.h"
#include "../utils/string_array.h"
#include "../utils/test_helpers.h"
#include "../utils/utf8.h"
#include "../utils/utils.h"
#include "../background.h"
#include "../bookmarks.h"
#include "../filelist.h"
#include "../macros.h"
#include "../running.h"
#include "../search.h"
#include "../status.h"
#include "../ui.h"

static int prompt_error_msg_internalv(const char title[], const char format[],
		int prompt_skip, va_list pa);
static int prompt_error_msg_internal(const char title[], const char message[],
		int prompt_skip);
TSTATIC char * parse_spec(const char spec[], int *line_num);
static void open_selected_file(const char path[], int line_num);
static void navigate_to_selected_file(FileView *view, const char path[]);
static void normalize_top(menu_info *m);
static void append_to_string(char **str, const char suffix[]);
static char * expand_tabulation_a(const char line[], size_t tab_stops);
static size_t chars_in_str(const char s[], char c);
static void redraw_error_msg(const char title_arg[], const char message_arg[],
		int prompt_skip);

static void
show_position_in_menu(menu_info *m)
{
	char pos_buf[POS_WIN_WIDTH + 1];
	snprintf(pos_buf, sizeof(pos_buf), " %d-%d ", m->pos + 1, m->len);

	ui_pos_window_set(pos_buf);
}

void
remove_current_item(menu_info *m)
{
	clean_menu_position(m);

	remove_from_string_array(m->items, m->len, m->pos);
	if(m->matches != NULL)
	{
		if(m->matches[m->pos])
			m->matching_entries--;
		memmove(m->matches + m->pos, m->matches + m->pos + 1,
				sizeof(int)*((m->len - 1) - m->pos));
	}
	m->len--;
	draw_menu(m);

	move_to_menu_pos(m->pos, m);
}

void
clean_menu_position(menu_info *m)
{
	int x, z;
	int off = 0;
	char * buf = (char *)NULL;
	col_attr_t col;
	int type = MENU_COLOR;

	x = getmaxx(menu_win) + get_utf8_overhead(m->items[m->pos]);

	buf = malloc(x + 2);

	/* TODO: check if this can ever be false. */
	if(m->items[m->pos] != NULL)
	{
		z = m->hor_pos;
		while(z-- > 0 && m->items[m->pos][off] != '\0')
		{
			size_t l = get_char_width(m->items[m->pos] + off);
			off += l;
			x -= l - 1;
		}
		snprintf(buf, x, " %s", m->items[m->pos] + off);
	}
	else
	{
		buf[0] = '\0';
	}

	for(z = 0; buf[z] != '\0'; z++)
		if(buf[z] == '\t')
			buf[z] = ' ';

	for(z = strlen(buf); z < x; z++)
		buf[z] = ' ';

	buf[x] = ' ';
	buf[x + 1] = '\0';

	col = cfg.cs.color[WIN_COLOR];

	if(cfg.hl_search && m->matches != NULL && m->matches[m->pos])
	{
		mix_colors(&col, &cfg.cs.color[SELECTED_COLOR]);
		type = SELECTED_COLOR;
	}

	init_pair(DCOLOR_BASE + type, col.fg, col.bg);
	wattrset(menu_win, COLOR_PAIR(type + DCOLOR_BASE) | col.attr);

	checked_wmove(menu_win, m->current, 1);
	if(get_screen_string_length(m->items[m->pos] + off) > getmaxx(menu_win) - 4)
	{
		size_t len = get_normal_utf8_string_widthn(buf,
				getmaxx(menu_win) - 3 - 4 + 1);
		memset(buf + len, ' ', strlen(buf) - len);
		buf[len + 3] = '\0';
		wprint(menu_win, buf);
		mvwaddstr(menu_win, m->current, getmaxx(menu_win) - 5, "...");
	}
	else
	{
		size_t len = get_normal_utf8_string_widthn(buf, getmaxx(menu_win) - 4 + 1);
		buf[len] = '\0';
		wprint(menu_win, buf);
	}
	waddstr(menu_win, " ");

	wattroff(menu_win, COLOR_PAIR(type + DCOLOR_BASE) | col.attr);

	free(buf);
}

void
show_error_msg(const char title[], const char message[])
{
	(void)prompt_error_msg_internal(title, message, 0);
}

void
show_error_msgf(const char title[], const char format[], ...)
{
	va_list pa;

	va_start(pa, format);
	(void)prompt_error_msg_internalv(title, format, 0, pa);
	va_end(pa);
}

int
prompt_error_msg(const char title[], const char message[])
{
	return prompt_error_msg_internal(title, message, 1);
}

int
prompt_error_msgf(const char title[], const char format[], ...)
{
	int result;
	va_list pa;

	va_start(pa, format);
	result = prompt_error_msg_internalv(title, format, 1, pa);
	va_end(pa);

	return result;
}

/* Just a varargs wrapper over prompt_error_msg_internal. */
static int
prompt_error_msg_internalv(const char title[], const char format[],
		int prompt_skip, va_list pa)
{
	char buf[2048];
	vsnprintf(buf, sizeof(buf), format, pa);
	return prompt_error_msg_internal(title, buf, prompt_skip);
}

/* Internal function for displaying error messages to a user.  Automatically
 * skips whitespace in front of the message and does nothing for empty messages
 * (due to skipping whitespace-only are counted as empty). When the prompt_skip
 * isn't zero, asks user about successive messages.  Returns non-zero if all
 * successive messages should be skipped, otherwise zero is returned. */
static int
prompt_error_msg_internal(const char title[], const char message[],
		int prompt_skip)
{
	static int skip_until_started;
	int key;

	if(curr_stats.load_stage == 0)
		return 1;
	if(curr_stats.load_stage < 2 && skip_until_started)
		return 1;

	message = skip_whitespace(message);
	if(*message == '\0')
	{
		return 0;
	}

	curr_stats.errmsg_shown = 1;

	redraw_error_msg(title, message, prompt_skip);

	do
		key = wgetch(error_win);
	while(key != 13 && (!prompt_skip || key != 3)); /* ascii Return, Ctrl-c */

	if(curr_stats.load_stage < 2)
		skip_until_started = key == 3;

	werase(error_win);
	wrefresh(error_win);

	curr_stats.errmsg_shown = 0;

	modes_update();
	if(curr_stats.need_update != UT_NONE)
		modes_redraw();

	return key == 3;
}

void
init_menu_info(menu_info *m, int menu_type, char empty_msg[])
{
	m->top = 0;
	m->current = 1;
	m->len = 0;
	m->pos = 0;
	m->hor_pos = 0;
	m->win_rows = getmaxy(menu_win);
	m->type = menu_type;
	m->match_dir = NONE;
	m->matching_entries = 0;
	m->matches = NULL;
	m->regexp = NULL;
	m->title = NULL;
	m->args = NULL;
	m->items = NULL;
	m->data = NULL;
	m->key_handler = NULL;
	m->extra_data = 0;
	m->execute_handler = NULL;
	m->empty_msg = empty_msg;
}

void
reset_popup_menu(menu_info *m)
{
	free(m->args);
	/* Menu elements don't always have data associated with them.  That's why we
	 * need this check. */
	if(m->data != NULL)
	{
		free_string_array(m->data, m->len);
	}
	free_string_array(m->items, m->len);
	free(m->regexp);
	free(m->matches);
	free(m->title);
	free(m->empty_msg);

	werase(menu_win);
}

void
setup_menu(void)
{
	scrollok(menu_win, FALSE);
	curs_set(FALSE);
	werase(menu_win);
	werase(status_bar);
	werase(pos_win);
	wrefresh(status_bar);
	wrefresh(pos_win);
}

void
move_to_menu_pos(int pos, menu_info *m)
{
	/* TODO: refactor this function move_to_menu_pos() */

	int redraw = 0;
	int x, z;
	char *buf = NULL;
	col_attr_t col;
	int off = 0;

	pos = MIN(m->len - 1, MAX(0, pos));
	if(pos < 0)
		return;

	normalize_top(m);

	if(pos > get_last_visible_line(m))
	{
		m->top = pos - (m->win_rows - 2 - 1);
		redraw = 1;
	}
	else if(pos < m->top)
	{
		m->top = pos;
		redraw = 1;
	}

	if(cfg.scroll_off > 0)
	{
		int s = MIN(DIV_ROUND_UP(m->win_rows - 2, 2), cfg.scroll_off);
		if(pos - m->top < s && m->top > 0)
		{
			m->top -= s - (pos - m->top);
			normalize_top(m);
			redraw = 1;
		}
		if(pos > get_last_visible_line(m) - s)
		{
			m->top += s - (get_last_visible_line(m) - pos);
			normalize_top(m);
			redraw = 1;
		}
	}

	m->current = 1 + (pos - m->top);

	if(redraw)
		draw_menu(m);

	x = getmaxx(menu_win) + get_utf8_overhead(m->items[pos]);
	buf = malloc(x + 2);
	if(buf == NULL)
		return;
	/* TODO: check if this can be false. */
	if(m->items[pos] != NULL)
	{
		z = m->hor_pos;
		while(z-- > 0 && m->items[pos][off] != '\0')
		{
			size_t l = get_char_width(m->items[pos] + off);
			off += l;
			x -= l - 1;
		}

		snprintf(buf, x, " %s", m->items[pos] + off);
	}
	else
	{
		buf[0] = '\0';
	}

	for(z = 0; buf[z] != '\0'; z++)
		if(buf[z] == '\t')
			buf[z] = ' ';

	for(z = strlen(buf); z < x; z++)
		buf[z] = ' ';

	buf[x] = ' ';
	buf[x + 1] = '\0';

	col = cfg.cs.color[WIN_COLOR];

	if(cfg.hl_search && m->matches != NULL && m->matches[pos])
		mix_colors(&col, &cfg.cs.color[SELECTED_COLOR]);

	mix_colors(&col, &cfg.cs.color[CURR_LINE_COLOR]);

	init_pair(DCOLOR_BASE + MENU_CURRENT_COLOR, col.fg, col.bg);
	wattrset(menu_win, COLOR_PAIR(DCOLOR_BASE + MENU_CURRENT_COLOR) | col.attr);

	checked_wmove(menu_win, m->current, 1);
	if(get_screen_string_length(m->items[pos] + off) > getmaxx(menu_win) - 4)
	{
		size_t len = get_normal_utf8_string_widthn(buf,
				getmaxx(menu_win) - 3 - 4 + 1);
		memset(buf + len, ' ', strlen(buf) - len);
		buf[len + 3] = '\0';
		wprint(menu_win, buf);
		mvwaddstr(menu_win, m->current, getmaxx(menu_win) - 5, "...");
	}
	else
	{
		size_t len = get_normal_utf8_string_widthn(buf, getmaxx(menu_win) - 4 + 1);
		buf[len] = '\0';
		wprint(menu_win, buf);
	}
	waddstr(menu_win, " ");

	wattroff(menu_win, COLOR_PAIR(DCOLOR_BASE + MENU_CURRENT_COLOR) | col.attr);

	m->pos = pos;
	free(buf);
	show_position_in_menu(m);
}

void
redraw_menu(menu_info *m)
{
	resize_for_menu_like();
	m->win_rows = getmaxy(menu_win);

	draw_menu(m);
	move_to_menu_pos(m->pos, m);
	wrefresh(menu_win);
}

void
goto_selected_file(FileView *view, const char spec[], int try_open)
{
	char *path_buf;
	int line_num;

	path_buf = parse_spec(spec, &line_num);
	if(path_buf == NULL)
	{
		show_error_msg("Memory Error", "Unable to allocate enough memory");
		return;
	}

	if(access(path_buf, F_OK) == 0)
	{
		if(try_open)
		{
			open_selected_file(path_buf, line_num);
		}
		else
		{
			navigate_to_selected_file(view, path_buf);
		}
	}
	else
	{
		show_error_msgf("Missing file", "File \"%s\" doesn't exist", path_buf);
	}

	free(path_buf);
}

/* Extracts path and line number from the spec (1 when absent from the spec).
 * Returns path and sets *line_num to line number, otherwise NULL is
 * returned. */
TSTATIC char *
parse_spec(const char spec[], int *line_num)
{
	char *path_buf;
	const char *colon;
	int colon_lookup_offset = 0;
	const size_t bufs_len = 2 + strlen(spec) + 1 + 1;

	path_buf = malloc(bufs_len);
	if(path_buf == NULL)
	{
		return NULL;
	}

	if(is_path_absolute(spec))
	{
		path_buf[0] = '\0';
	}
	else
	{
		copy_str(path_buf, bufs_len, "./");
	}

#ifdef _WIN32
	if(is_path_absolute(spec))
	{
		colon_lookup_offset = 2;
	}
#endif

	colon = strchr(spec + colon_lookup_offset, ':');
	if(colon != NULL)
	{
		strncat(path_buf, spec, colon - spec);
		*line_num = atoi(colon + 1);
	}
	else
	{
		strcat(path_buf, spec);
		*line_num = 1;
	}

	chomp(path_buf);

#ifdef _WIN32
	to_forward_slash(path_buf);
#endif

	return path_buf;
}

/* Opens file specified by its path on the given line number. */
static void
open_selected_file(const char path[], int line_num)
{
	if(access(path, R_OK) == 0)
	{
		(void)view_file(path, line_num, -1, 1);
	}
	else
	{
		show_error_msgf("Can't read file", "File \"%s\" is not readable", path);
	}
}

/* Navigates the view to a given dir/file combination specified by the path. */
static void
navigate_to_selected_file(FileView *view, const char path[])
{
	/* Check whether target path is directory while we don't change current
	 * working directory by invoking change_directory() function below. */
	const int dst_is_dir = is_dir(path);

	char name[NAME_MAX];
	char *dir = strdup(path);
	char *const last_slash = find_slashr(dir);

	if(last_slash == NULL)
	{
		copy_str(name, sizeof(name), dir);
	}
	else
	{
		*last_slash = '\0';
		copy_str(name, sizeof(name), last_slash + 1);
	}

	if(change_directory(view, dir) >= 0)
	{
		status_bar_message("Finding the correct directory...");
		wrefresh(status_bar);

		load_dir_list(view, 0);

		if(dst_is_dir)
		{
			strcat(name, "/");
		}
		(void)ensure_file_is_selected(view, name);
	}
	else
	{
		show_error_msgf("Invalid path", "Cannot change dir to \"%s\"", dir);
	}

	free(dir);
}

void
goto_selected_directory(FileView *view, menu_info *m)
{
	if(!cfg.auto_ch_pos)
	{
		clean_positions_in_history(curr_view);
		curr_stats.ch_pos = 0;
	}
	navigate_to(view, m->items[m->pos]);
	if(!cfg.auto_ch_pos)
	{
		curr_stats.ch_pos = 1;
	}
}

void
draw_menu(menu_info *m)
{
	int i;
	int win_len;
	int x, y;

	getmaxyx(menu_win, y, x);
	win_len = x;
	werase(menu_win);

	normalize_top(m);

	x = m->top;

	box(menu_win, 0, 0);
	wattron(menu_win, A_BOLD);
	checked_wmove(menu_win, 0, 3);
	wprint(menu_win, m->title);
	wattroff(menu_win, A_BOLD);

	for(i = 1; x < m->len; i++, x++)
	{
		int z, off;
		char *buf;
		char *ptr = NULL;
		col_attr_t col;
		int type = WIN_COLOR;

		chomp(m->items[x]);
		if((ptr = strchr(m->items[x], '\n')) || (ptr = strchr(m->items[x], '\r')))
			*ptr = '\0';

		col = cfg.cs.color[WIN_COLOR];

		if(cfg.hl_search && m->matches != NULL && m->matches[x])
		{
			mix_colors(&col, &cfg.cs.color[SELECTED_COLOR]);
			type = SELECTED_COLOR;
		}

		init_pair(DCOLOR_BASE + type, col.fg, col.bg);
		wattron(menu_win, COLOR_PAIR(DCOLOR_BASE + type) | col.attr);

		z = m->hor_pos;
		off = 0;
		while(z-- > 0 && m->items[x][off] != '\0')
		{
			size_t l = get_char_width(m->items[x] + off);
			off += l;
		}

		buf = strdup(m->items[x] + off);
		for(z = 0; buf[z] != '\0'; z++)
			if(buf[z] == '\t')
				buf[z] = ' ';

		checked_wmove(menu_win, i, 2);
		if(get_screen_string_length(buf) > win_len - 4)
		{
			size_t len = get_normal_utf8_string_widthn(buf, win_len - 3 - 4);
			memset(buf + len, ' ', strlen(buf) - len);
			buf[len + 3] = '\0';
			wprint(menu_win, buf);
			mvwaddstr(menu_win, i, win_len - 5, "...");
		}
		else
		{
			const size_t len = get_normal_utf8_string_widthn(buf, win_len - 4);
			buf[len] = '\0';
			wprint(menu_win, buf);
		}
		waddstr(menu_win, " ");

		free(buf);

		wattroff(menu_win, COLOR_PAIR(DCOLOR_BASE + type) | col.attr);

		if(i + 3 > y)
			break;
	}
}

/* Ensures that value of m->top lies in a correct range. */
static void
normalize_top(menu_info *m)
{
	m->top = MAX(0, MIN(m->len - (m->win_rows - 2), m->top));
}

int
capture_output_to_menu(FileView *view, const char cmd[], menu_info *m)
{
	FILE *file, *err;
	char *line = NULL;
	int x;
	pid_t pid;

	LOG_INFO_MSG("Capturing output of the command to a menu: %s", cmd);

	pid = background_and_capture((char *)cmd, &file, &err);
	if(pid == (pid_t)-1)
	{
		show_error_msgf("Trouble running command", "Unable to run: %s", cmd);
		return 0;
	}

	show_progress("", 0);

	ui_cancellation_reset();
	ui_cancellation_enable();

	wait_for_data_from(pid, file, 0);

	x = 0;
	while((line = read_line(file, line)) != NULL)
	{
		char *expanded_line;
		show_progress("Loading menu", 1000);
		m->items = realloc(m->items, sizeof(char *)*(x + 1));
		expanded_line = expand_tabulation_a(line, cfg.tab_stop);
		if(expanded_line != NULL)
		{
			m->items[x++] = expanded_line;
		}

		wait_for_data_from(pid, file, 0);
	}
	m->len = x;

	ui_cancellation_disable();

	fclose(file);
	print_errors(err);

	if(ui_cancellation_requested())
	{
		append_to_string(&m->title, "(cancelled) ");
		append_to_string(&m->empty_msg, " (cancelled)");
	}

	return display_menu(m, view);
}

/* Replaces *str with a copy of the with string extended by the suffix.  *str
 * can be NULL in which case it's treated as empty string. equal to the with (then function does nothing).  Returns non-zero if memory allocation
 * failed. */
static void
append_to_string(char **str, const char suffix[])
{
	const char *const non_null_str = (*str == NULL) ? "" : *str;
	char *const appended_str = format_str("%s%s", non_null_str, suffix);
	if(appended_str != NULL)
	{
		free(*str);
		*str = appended_str;
	}
}

/* Clones the line replacing all occurrences of horizontal tabulation character
 * with appropriate number of spaces.  The tab_stops parameter shows how many
 * character position are taken by one tabulation.  Returns newly allocated
 * string. */
static char *
expand_tabulation_a(const char line[], size_t tab_stops)
{
	const size_t tab_count = chars_in_str(line, '\t');
	const size_t extra_line_len = tab_count*tab_stops;
	const size_t expanded_line_len = (strlen(line) - tab_count) + extra_line_len;
	char *const expanded_line = malloc(expanded_line_len + 1);

	if(expanded_line != NULL)
	{
		const char *const end = expand_tabulation(line, (size_t)-1, tab_stops,
				expanded_line);
		assert(*end == '\0' && "The line should be processed till the end");
	}

	return expanded_line;
}

/* Returns number of c char occurrences in the s string. */
static size_t
chars_in_str(const char s[], char c)
{
	size_t char_count = 0;
	while(*s != '\0')
	{
		if(*s++ == c)
		{
			char_count++;
		}
	}
	return char_count;
}

int
display_menu(menu_info *m, FileView *view)
{
	if(m->len < 1)
	{
		status_bar_message(m->empty_msg);
		reset_popup_menu(m);
		return 1;
	}
	else
	{
		setup_menu();
		draw_menu(m);
		move_to_menu_pos(m->pos, m);
		enter_menu_mode(m, view);
		return 0;
	}
}

int
query_user_menu(char *title, char *message)
{
	int key;
	char *dup = strdup(message);

	curr_stats.errmsg_shown = 2;

	redraw_error_msg(title, message, 0);

	do
	{
		key = wgetch(error_win);
	}
	while(key != 'y' && key != 'n' && key != ERR);

	free(dup);

	curr_stats.errmsg_shown = 0;

	werase(error_win);
	wrefresh(error_win);

	touchwin(stdscr);

	update_all_windows();

	if(curr_stats.need_update != UT_NONE)
		update_screen(UT_FULL);

	return key == 'y';
}

void
redraw_error_msg_window(void)
{
	redraw_error_msg(NULL, NULL, 0);
}

/* Draws error message on the screen or redraws the last message when both
 * title_arg and message_arg are NULL. */
static void
redraw_error_msg(const char title_arg[], const char message_arg[],
		int prompt_skip)
{
	/* TODO: refactor this function redraw_error_msg() */

	static const char *title;
	static const char *message;
	static int ctrl_c;

	int sx, sy;
	int x, y;
	int z;
	const char *text;

	if(title_arg != NULL && message_arg != NULL)
	{
		title = title_arg;
		message = message_arg;
		ctrl_c = prompt_skip;
	}

	assert(message != NULL);

	curs_set(FALSE);
	werase(error_win);

	getmaxyx(stdscr, sy, sx);
	getmaxyx(error_win, y, x);

	z = strlen(message);
	if(z <= x - 2 && strchr(message, '\n') == NULL)
	{
		y = 6;
		wresize(error_win, y, x);
		mvwin(error_win, (sy - y)/2, (sx - x)/2);
		checked_wmove(error_win, 2, (x - z)/2);
		wprint(error_win, message);
	}
	else
	{
		int i;
		int cy = 2;
		i = 0;
		while(i < z)
		{
			int j;
			char buf[x - 2 + 1];

			snprintf(buf, sizeof(buf), "%s", message + i);

			for(j = 0; buf[j] != '\0'; j++)
				if(buf[j] == '\n')
					break;

			if(buf[j] != '\0')
				i++;
			buf[j] = '\0';
			i += j;

			if(buf[0] == '\0')
				continue;

			y = cy + 4;
			mvwin(error_win, (sy - y)/2, (sx - x)/2);
			wresize(error_win, y, x);

			checked_wmove(error_win, cy++, 1);
			wprint(error_win, buf);
		}
	}

	box(error_win, 0, 0);
	if(title[0] != '\0')
		mvwprintw(error_win, 0, (x - strlen(title) - 2)/2, " %s ", title);

	if(curr_stats.errmsg_shown == 1)
	{
		if(ctrl_c)
		{
			text = "Press Return to continue or Ctrl-C to skip other error messages";
		}
		else
		{
			text = "Press Return to continue";
		}
	}
	else
	{
		text = "Enter [y]es or [n]o";
	}
	mvwaddstr(error_win, y - 2, (x - strlen(text))/2, text);
}

void
print_errors(FILE *ef)
{
	char linebuf[160];
	char buf[sizeof(linebuf)*5];

	if(ef == NULL)
		return;

	buf[0] = '\0';
	while(fgets(linebuf, sizeof(linebuf), ef) == linebuf)
	{
		if(linebuf[0] == '\n')
			continue;
		if(strlen(buf) + strlen(linebuf) + 1 >= sizeof(buf))
		{
			int skip = (prompt_error_msg("Background Process Error", buf) != 0);
			buf[0] = '\0';
			if(skip)
				break;
		}
		strcat(buf, linebuf);
	}

	if(buf[0] != '\0')
		show_error_msg("Background Process Error", buf);

	fclose(ef);
}

char *
get_cmd_target(void)
{
	return (curr_view->selected_files > 0) ?
		expand_macros("%f", NULL, NULL, 1) : strdup(".");
}

KHandlerResponse
filelist_khandler(menu_info *m, const wchar_t keys[])
{
	if(wcscmp(keys, L"gf") == 0)
	{
		goto_selected_file(curr_view, m->items[m->pos], 0);
		return KHR_CLOSE_MENU;
	}
	else if(wcscmp(keys, L"e") == 0)
	{
		goto_selected_file(curr_view, m->items[m->pos], 1);
		return KHR_REFRESH_WINDOW;
	}
	return KHR_UNHANDLED;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
