// basic headers
#define _GNU_SOURCE /* really just for strndup */

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

// this doesn't really belong here, but it was easiest
#ifdef HAVE_MAGICK
#define BUILDSPEC_MAGICK " imagemagick"
#else
#ifdef HAVE_GMAGICK
#define BUILDSPEC_MAGICK " graphicsmagick"
#else
#define BUILDSPEC_MAGICK ""
#endif
#endif

#ifdef HAVE_GETOPT_LONG
#define BUILDSPEC_GETOPT " gnugetopt"
#else
#define BUILDSPEC_GETOPT ""
#endif

#ifdef HAVE_ICONV
#define BUILDSPEC_ICONV " iconv"
#else
#define BUILDSPEC_ICONV ""
#endif

#ifdef HAVE_FREETYPE
#define BUILDSPEC_FREETYPE " freetype"
#else
#define BUILDSPEC_FREETYPE ""
#endif

#ifdef HAVE_FRIBIDI
#define BUILDSPEC_FRIBIDI " fribidi"
#ifndef HAVE_FRIBIDI2
#define FriBidiParType FriBidiCharType
#endif
#else
#define BUILDSPEC_FRIBIDI ""
#endif

#ifdef HAVE_FONTCONFIG
#define BUILDSPEC_FONTCONFIG " fontconfig"
#else
#define BUILDSPEC_FONTCONFIG ""
#endif

#define BUILDSPEC BUILDSPEC_GETOPT BUILDSPEC_MAGICK BUILDSPEC_ICONV BUILDSPEC_FREETYPE BUILDSPEC_FRIBIDI BUILDSPEC_FONTCONFIG

#ifdef HAVE_ICONV

#define ICONV_NULL ((iconv_t)-1)
extern const char * default_charset;
  /* the name of the default character set to use, depending on user's locale settings */

#endif /*HAVE_ICONV*/

#ifndef HAVE_STRNDUP
char * strndup
  (
    const char * s,
    size_t n
  );
#endif

#define PACKAGE_HEADER(x) PACKAGE_NAME "::" x ", version " PACKAGE_VERSION ".\nBuild options:" BUILDSPEC "\nSend bug reports to <" PACKAGE_BUGREPORT ">\n\n"


enum {VF_NONE=0,VF_NTSC=1,VF_PAL=2}; /* values for videodesc.vformat in da-internal.h as well as other uses */

