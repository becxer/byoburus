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

#include "utils_nix.h"
#include "utils_int.h"

#include <sys/stat.h> /* S_* */
#include <sys/types.h> /* gid_t mode_t pid_t uid_t */
#include <fcntl.h> /* O_RDONLY open() close() */
#include <grp.h> /* getgrnam() */
#include <pwd.h> /* getpwnam() */
#include <unistd.h> /* dup2() getpid() */

#include <assert.h> /* assert() */
#include <ctype.h> /* isdigit() */
#include <errno.h> /* EINTR errno */
#include <signal.h> /* signal() SIGINT SIGTSTP SIGCHLD SIG_DFL sigset_t
                       sigemptyset() sigaddset() sigprocmask() SIG_BLOCK
                       SIG_UNBLOCK */
#include <stddef.h> /* NULL size_t */
#include <stdio.h> /* snprintf() */
#include <stdlib.h> /* atoi() */
#include <string.h> /* strchr() strlen() strncmp() */

#include "../cfg/config.h"
#include "fs_limits.h"
#include "log.h"
#include "mntent.h" /* mntent setmntent() getmntent() endmntent() */
#include "path.h"
#include "str.h"
#include "utils.h"

/* Types of mount point information for get_mount_point_traverser_state. */
typedef enum
{
	MI_MOUNT_POINT, /* Path to the mount point. */
	MI_FS_TYPE,     /* File system name. */
}
mntinfo;

/* State for get_mount_point() traverser. */
typedef struct
{
	mntinfo type;     /* Type of information to put into the buffer. */
	const char *path; /* Path whose mount point we're looking for. */
	size_t buf_len;   /* Output buffer length. */
	char *buf;        /* Output buffer. */
	size_t curr_len;  /* Max length among all found points (len of buf string). */
}
get_mount_point_traverser_state;

static void process_cancel_request(pid_t pid);
static int get_mount_info_traverser(struct mntent *entry, void *arg);
static int begins_with_list_item(const char pattern[], const char list[]);

void
pause_shell(void)
{
	run_in_shell_no_cls(PAUSE_CMD);
}

int
run_in_shell_no_cls(char command[])
{
	typedef void (*sig_handler)(int);

	int pid;
	int result;
	extern char **environ;
	sig_handler sigtstp_handler;

	if(command == NULL)
		return 1;

	sigtstp_handler = signal(SIGTSTP, SIG_DFL);

	/* We need to block SIGCHLD signal.  One can't just set it to SIG_DFL, because
	 * it will possibly cause missing of SIGCHLD from a background process
	 * (job). */
	(void)set_sigchld(1);

	pid = fork();
	if(pid == -1)
	{
		signal(SIGTSTP, sigtstp_handler);
		(void)set_sigchld(0);
		return -1;
	}
	if(pid == 0)
	{
		char *args[4];

		signal(SIGTSTP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		(void)set_sigchld(0);

		args[0] = cfg.shell;
		args[1] = "-c";
		args[2] = command;
		args[3] = NULL;
		execve(cfg.shell, args, environ);
		exit(127);
	}

	result = get_proc_exit_status(pid);

	signal(SIGTSTP, sigtstp_handler);
	(void)set_sigchld(0);

	return result;
}

void
recover_after_shellout(void)
{
	/* Do nothing.  No need to recover anything on this platform. */
}

void
wait_for_data_from(pid_t pid, FILE *f, int fd)
{
	const struct timeval ts_init = { .tv_sec = 0, .tv_usec = 1000 };
	struct timeval ts;

	fd_set read_ready;
	FD_ZERO(&read_ready);

	fd = (f != NULL) ? fileno(f) : fd;

	do
	{
		process_cancel_request(pid);
		ts = ts_init;
		FD_SET(fd, &read_ready);
	}
	while(select(fd + 1, &read_ready, NULL, NULL, &ts) == 0);

	/* Inform other parts of the application that cancellation took place. */
	if(errno == EINTR)
	{
		ui_cancellation_request();
	}
}

int
set_sigchld(int block)
{
	const int action = block ? SIG_BLOCK : SIG_UNBLOCK;
	sigset_t sigchld_mask;

	return sigemptyset(&sigchld_mask) == -1
	    || sigaddset(&sigchld_mask, SIGCHLD) == -1
	    || sigprocmask(action, &sigchld_mask, NULL) == -1;
}

/* Checks whether cancelling of current operation is requested and sends SIGINT
 * to process specified by its process id to request cancellation. */
static void
process_cancel_request(pid_t pid)
{
	if(ui_cancellation_requested())
	{
		if(kill(pid, SIGINT) != 0)
		{
			LOG_SERROR_MSG(errno, "Failed to send SIGINT to " PRINTF_PID_T, pid);
		}
	}
}

int
get_proc_exit_status(pid_t pid)
{
	do
	{
		int status;
		if(waitpid(pid, &status, 0) == -1)
		{
			if(errno != EINTR)
			{
				LOG_SERROR_MSG(errno, "waitpid()");
				return -1;
			}
		}
		else
		{
			return status;
		}
	}
	while(1);
}

/* if err == 1 then use stderr and close stdin and stdout */
void _gnuc_noreturn
run_from_fork(int pipe[2], int err, char *cmd)
{
	char *args[4];
	int nullfd;

	/* Redirect stderr or stdout to write end of pipe. */
	if(dup2(pipe[1], err ? STDERR_FILENO : STDOUT_FILENO) == -1)
	{
		exit(1);
	}
	close(pipe[0]);        /* Close read end of pipe. */
	close(STDIN_FILENO);
	close(err ? STDOUT_FILENO : STDERR_FILENO);

	/* Send stdout, stdin to /dev/null */
	if((nullfd = open("/dev/null", O_RDONLY)) != -1)
	{
		if(dup2(nullfd, STDIN_FILENO) == -1)
			exit(1);
		if(dup2(nullfd, err ? STDOUT_FILENO : STDERR_FILENO) == -1)
			exit(1);
	}

	args[0] = cfg.shell;
	args[1] = "-c";
	args[2] = cmd;
	args[3] = NULL;

	execvp(args[0], args);
	exit(1);
}

void
get_perm_string(char buf[], int len, mode_t mode)
{
	static const char *const perm_sets[] =
	{ "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" };
	int u, g, o;

	u = (mode & S_IRWXU) >> 6;
	g = (mode & S_IRWXG) >> 3;
	o = (mode & S_IRWXO);

	snprintf(buf, len, "-%s%s%s", perm_sets[u], perm_sets[g], perm_sets[o]);

	if(S_ISLNK(mode))
		buf[0] = 'l';
	else if(S_ISDIR(mode))
		buf[0] = 'd';
	else if(S_ISBLK(mode))
		buf[0] = 'b';
	else if(S_ISCHR(mode))
		buf[0] = 'c';
	else if(S_ISFIFO(mode))
		buf[0] = 'p';
	else if(S_ISSOCK(mode))
		buf[0] = 's';

	if(mode & S_ISVTX)
		buf[9] = (buf[9] == '-') ? 'T' : 't';
	if(mode & S_ISGID)
		buf[6] = (buf[6] == '-') ? 'S' : 's';
	if(mode & S_ISUID)
		buf[3] = (buf[3] == '-') ? 'S' : 's';
}

int
is_on_slow_fs(const char full_path[])
{
	char fs_name[PATH_MAX];
	get_mount_point_traverser_state state =
	{
		.type = MI_FS_TYPE,
		.path = full_path,
		.buf_len = sizeof(fs_name),
		.buf = fs_name,
		.curr_len = 0UL,
	};

	/* Empty list optimization. */
	if(cfg.slow_fs_list[0] == '\0')
	{
		return 0;
	}

	if(traverse_mount_points(&get_mount_info_traverser, &state) == 0)
	{
		if(state.curr_len > 0)
		{
			return begins_with_list_item(fs_name, cfg.slow_fs_list);
		}
	}
	return 0;
}

int
get_mount_point(const char path[], size_t buf_len, char buf[])
{
	get_mount_point_traverser_state state =
	{
		.type = MI_MOUNT_POINT,
		.path = path,
		.buf_len = buf_len,
		.buf = buf,
		.curr_len = 0UL,
	};
	return traverse_mount_points(&get_mount_info_traverser, &state);
}

/* traverse_mount_points client that gets mount point info for a given path. */
static int
get_mount_info_traverser(struct mntent *entry, void *arg)
{
	get_mount_point_traverser_state *const state = arg;
	if(path_starts_with(state->path, entry->mnt_dir))
	{
		const size_t new_len = strlen(entry->mnt_dir);
		if(new_len > state->curr_len)
		{
			state->curr_len = new_len;
			switch(state->type)
			{
				case MI_MOUNT_POINT:
					copy_str(state->buf, state->buf_len, entry->mnt_dir);
					break;
				case MI_FS_TYPE:
					copy_str(state->buf, state->buf_len, entry->mnt_type);
					break;

				default:
					assert(0 && "Unknown mount information type.");
					break;
			}
		}
	}
	return 0;
}

int
traverse_mount_points(mptraverser client, void *arg)
{
	FILE *f;
	struct mntent *ent;

	if((f = setmntent("/etc/mtab", "r")) == NULL)
	{
		return 1;
	}

	while((ent = getmntent(f)) != NULL)
	{
		client(ent, arg);
	}

	endmntent(f);
	return 0;
}

static int
begins_with_list_item(const char pattern[], const char list[])
{
	const char *p = list - 1;

	do
	{
		char buf[128];
		const char *t;
		size_t len;

		t = p + 1;
		p = strchr(t, ',');
		if(p == NULL)
			p = t + strlen(t);

		len = snprintf(buf, MIN(p - t + 1, sizeof(buf)), "%s", t);
		if(len != 0 && strncmp(pattern, buf, len) == 0)
			return 1;
	}
	while(*p != '\0');
	return 0;
}

unsigned int
get_pid(void)
{
	return getpid();
}

int
get_uid(const char user[], uid_t *uid)
{
	if(isdigit(user[0]))
	{
		*uid = atoi(user);
	}
	else
	{
		struct passwd *p;

		p = getpwnam(user);
		if(p == NULL)
			return 1;

		*uid = p->pw_uid;
	}
	return 0;
}

int
get_gid(const char group[], gid_t *gid)
{
	if(isdigit(group[0]))
	{
		*gid = atoi(group);
	}
	else
	{
		struct group *g;

		g = getgrnam(group);
		if(g == NULL)
			return 1;

		*gid = g->gr_gid;
	}
	return 0;
}

int
S_ISEXE(mode_t mode)
{
	return ((S_IXUSR | S_IXGRP | S_IXOTH) & mode);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
