/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* load libX11 dynamically */
/* #undef DYN_X11 */

/* parsing of .desktop files */
#define ENABLE_DESKTOP_FILES 1

/* enables extended keys (arrows etc.) */
#define ENABLE_EXTENDED_KEYS 1

/* executing commands remotely */
/* #undef ENABLE_REMOTE_CMDS */

/* GNU toolchain is installed */
#define GNU_TOOLCHAIN 1

/* Define to 1 if you have the declaration of `MAGIC_MIME_TYPE', and to 0 if
   you don't. */
/* #undef HAVE_DECL_MAGIC_MIME_TYPE */

/* Define if file program present */
#define HAVE_FILE_PROG 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* use gtk to determine mime type */
/* #undef HAVE_LIBGTK */

/* Define to 1 if you have the `magic' library (-lmagic). */
/* #undef HAVE_LIBMAGIC */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <mntent.h> header file. */
#define HAVE_MNTENT_H 1

/* set_escdelay() function is available. */
#define HAVE_SET_ESCDELAY_FUNC 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if `st_mtim' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIM 1

/* strverscmp() function is available. */
#define HAVE_STRVERSCMP_FUNC 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* use X11 to determine terminal emulator title */
#define HAVE_X11 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "vifm"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Data directory of the package. */
#define PACKAGE_DATA_DIR "/usr/local/share/vifm"

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Source directory of the package. */
#define PACKAGE_SOURCE_DIR "/home/becxer/byoburus/vifm-0.7.7"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* -n option is available for cp and mv */
#define SUPPORT_NO_CLOBBER 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "0.7.7"

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to 1 to enable wide functions of ncurses on OS X. */
/* #undef _XOPEN_SOURCE_EXTENDED */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */
