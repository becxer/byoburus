#include "seatest.h"

#include "../../src/engine/options.h"
#include "../../src/engine/parsing.h"
#include "../../src/utils/macros.h"

#include "asserts.h"

static const char cpoptions_charset[] = "abc";
static const char * cpoptions_vals = cpoptions_charset;

static const char * sort_enum[] =
{
	"ext",
	"name",
	"gid",
	"gname",
	"mode",
	"uid",
	"uname",
	"size",
	"atime",
	"ctime",
	"mtime",
};

static const char * vifminfo_set[] =
{
	"options",
	"filetypes",
	"commands",
	"bookmarks",
	"tui",
	"dhistory",
	"state",
	"cs",
};

static void
dummy_handler(OPT_OP op, optval_t val)
{
}

static void
setup(void)
{
	static int option_changed;
	optval_t val;

	init_options(&option_changed);

	val.str_val = "bc";
	add_option("cpoptions", "cpo", OPT_CHARSET, ARRAY_LEN(cpoptions_charset),
			&cpoptions_vals, dummy_handler, val);

	val.bool_val = 0;
	add_option("fastrun", "fr", OPT_BOOL, 0, NULL, dummy_handler, val);

	val.str_val = "fusehome-default";
	add_option("fusehome", "fh", OPT_STR, 0, NULL, dummy_handler, val);

	val.str_val = "%r/.vifm-Trash,$HOME/.vifm/Trash";
	add_option("trashdir", "fh", OPT_STRLIST, 0, NULL, dummy_handler, val);

	val.enum_item = 1;
	add_option("sort", "so", OPT_ENUM, ARRAY_LEN(sort_enum), sort_enum,
			&dummy_handler, val);

	val.int_val = 8;
	add_option("tabstop", "ts", OPT_INT, 0, NULL, &dummy_handler, val);

	val.set_items = 0x81;
	add_option("vifminfo", "", OPT_SET, ARRAY_LEN(vifminfo_set), vifminfo_set,
			&dummy_handler, val);
}

static void
teardown(void)
{
	clear_options();
}

static void
test_nothing_follows_fail(void)
{
	ASSERT_FAIL("&", PE_INVALID_EXPRESSION);
}

static void
test_space_follows_fail(void)
{
	ASSERT_FAIL("& ", PE_INVALID_EXPRESSION);
}

static void
test_number_follows_fail(void)
{
	ASSERT_FAIL("&1", PE_INVALID_EXPRESSION);
}

static void
test_wrong_option_name_fail(void)
{
	ASSERT_FAIL("&nosuchoption", PE_INVALID_EXPRESSION);
}

static void
test_correct_full_option_name_ok(void)
{
	ASSERT_INT_OK("&tabstop", 8);
}

static void
test_correct_short_option_name_ok(void)
{
	ASSERT_INT_OK("&ts", 8);
}

static void
test_concatenation_ok(void)
{
	ASSERT_OK("&ts.&ts", "88");
}

static void
test_all_fail(void)
{
	ASSERT_FAIL("&all", PE_INVALID_EXPRESSION);
}

static void
test_bool_option_ok(void)
{
	ASSERT_INT_OK("&fastrun", 0);
}

static void
test_int_option_ok(void)
{
	ASSERT_INT_OK("&tabstop", 8);
}

static void
test_str_option_ok(void)
{
	ASSERT_OK("&fusehome", "fusehome-default");
}

static void
test_strlist_option_ok(void)
{
	ASSERT_OK("&trashdir", "%r/.vifm-Trash,$HOME/.vifm/Trash");
}

static void
test_enum_option_ok(void)
{
	ASSERT_OK("&sort", "name");
}

static void
test_set_option_ok(void)
{
	ASSERT_OK("&vifminfo", "options,cs");
}

static void
test_charset_option_ok(void)
{
	ASSERT_OK("&cpoptions", "bc");
}

static void
test_very_long_option_name_fail(void)
{
	ASSERT_FAIL("&ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
			PE_INVALID_EXPRESSION);
}

void
ampersand_tests(void)
{
	test_fixture_start();

	fixture_setup(&setup);
	fixture_teardown(&teardown);

	run_test(test_nothing_follows_fail);
	run_test(test_space_follows_fail);
	run_test(test_number_follows_fail);
	run_test(test_wrong_option_name_fail);
	run_test(test_correct_full_option_name_ok);
	run_test(test_correct_short_option_name_ok);
	run_test(test_concatenation_ok);
	run_test(test_all_fail);
	run_test(test_bool_option_ok);
	run_test(test_int_option_ok);
	run_test(test_str_option_ok);
	run_test(test_strlist_option_ok);
	run_test(test_enum_option_ok);
	run_test(test_set_option_ok);
	run_test(test_charset_option_ok);
	run_test(test_very_long_option_name_fail);

	test_fixture_end();
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
