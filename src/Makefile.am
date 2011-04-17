
bin_PROGRAMS = mkinfo


AM_CPPFLAGS = -DSYSCONFDIR="\"$(sysconfdir)\"" \
    @XML_CPPFLAGS@ @MAGICK_CPPFLAGS@ @FREETYPE_CPPFLAGS@ @FRIBIDI_CFLAGS@ @FONTCONFIG_CFLAGS@
AM_CFLAGS = -Wall -Wunreachable-code

mkinfo_SOURCES = mkinfo.c common.h mkinfo.h mi-internal.h \
    dvdvm.h dvdvmy.h \
    dvdifo.c \
    dvdcli.c readxml.h \
    conffile.h compat.h rgb.h
mkinfo_LDADD = $(LIBICONV) $(XML_LIBS)


edit = sed \
    -e 's,@sysconfdir\@,$(sysconfdir),g' \
    -e 's,@PACKAGE_NAME\@,@PACKAGE_NAME@,g' \
    -e 's,@PACKAGE_VERSION\@,@PACKAGE_VERSION@,g' \
    -e 's,@PACKAGE_BUGREPORT\@,@PACKAGE_BUGREPORT@,g'

FLEX = flex
BISON = bison


