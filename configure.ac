AC_INIT(mkinfo,0.0.1)

AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_AUX_DIR(autotools)

AM_INIT_AUTOMAKE

AC_PROG_INSTALL

AC_SYS_LARGEFILE

AC_HEADER_STDBOOL

dnl AC_CHECK_HEADERS initializes CPP, so must appear outside of any conditionals
AC_CHECK_HEADERS( \
    getopt.h \
    io.h \
)


AC_CHECK_DECLS(O_BINARY, , , [ #include <fcntl.h> ] )

AC_OUTPUT(Makefile src/Makefile)
