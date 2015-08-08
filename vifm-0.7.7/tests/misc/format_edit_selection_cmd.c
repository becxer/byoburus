#include <stdlib.h>
#include <string.h>

#include "seatest.h"

#include "../../src/cfg/config.h"
#include "../../src/filelist.h"
#include "../../src/running.h"
#include "../../src/ui.h"

static void teardown_view(FileView *view);

static void
setup_lwin(void)
{
	strcpy(lwin.curr_dir, "/lwin");

	lwin.list_rows = 4;
	lwin.list_pos = 2;
	lwin.dir_entry = calloc(lwin.list_rows, sizeof(*lwin.dir_entry));
	lwin.dir_entry[0].name = strdup("lfile0");
	lwin.dir_entry[1].name = strdup("lfile1");
	lwin.dir_entry[2].name = strdup("lfile2");
	lwin.dir_entry[3].name = strdup("lfile3");

	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[2].selected = 1;
	lwin.selected_files = 2;
}

static void
setup_rwin(void)
{
	strcpy(rwin.curr_dir, "/rwin");

	rwin.list_rows = 6;
	rwin.list_pos = 5;
	rwin.dir_entry = calloc(rwin.list_rows, sizeof(*rwin.dir_entry));
	rwin.dir_entry[0].name = strdup("rfile0");
	rwin.dir_entry[1].name = strdup("rfile1");
	rwin.dir_entry[2].name = strdup("rfile2");
	rwin.dir_entry[3].name = strdup("rfile3");
	rwin.dir_entry[4].name = strdup("rfile4");
	rwin.dir_entry[5].name = strdup("rfile5");

	rwin.dir_entry[1].selected = 1;
	rwin.dir_entry[3].selected = 1;
	rwin.dir_entry[5].selected = 1;
	rwin.selected_files = 3;
}

static void
setup(void)
{
	setup_lwin();
	setup_rwin();

	curr_view = &lwin;
	other_view = &rwin;

	cfg.max_args = 8192;
	cfg.vi_command = strdup("vim -p");
	cfg.vi_x_command = strdup("");
}

static void
teardown(void)
{
	teardown_view(&lwin);
	teardown_view(&rwin);

	free(cfg.vi_command);
}

static void
teardown_view(FileView *view)
{
	int i;
	for(i = 0; i < view->list_rows; i++)
	{
		free(view->dir_entry[i].name);
	}
	free(view->dir_entry);
	view->list_rows = 0;
	view->selected_files = 0;
}

static void
test_selection(void)
{
	char *cmd;
	int bg;

	cmd = format_edit_selection_cmd(&bg);
	assert_string_equal("vim -p lfile0 lfile2", cmd);
	free(cmd);
}

void
format_edit_selection_cmd_tests(void)
{
	test_fixture_start();

	fixture_setup(setup);
	fixture_teardown(teardown);

	run_test(test_selection);

	test_fixture_end();
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
