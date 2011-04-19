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



#define PACKAGE_HEADER(x) PACKAGE_NAME "::" x ", version " PACKAGE_VERSION ".\nBuild options:" BUILDSPEC "\nSend bug reports to <" PACKAGE_BUGREPORT ">\n\n"


enum {VF_NONE=0,VF_NTSC=1,VF_PAL=2}; /* values for videodesc.vformat in da-internal.h as well as other uses */

