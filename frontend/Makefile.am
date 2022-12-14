# Makefile.am

# Copyright (C) 2011-2019 by Werner Lemberg.
#
# This file is part of the ttfautohint library, and may only be used,
# modified, and distributed under the terms given in `COPYING'.  By
# continuing to use, modify, or distribute this file you indicate that you
# have read `COPYING' and understand and accept it fully.
#
# The file `COPYING' mentioned in the previous paragraph is distributed
# with the ttfautohint library.

SUFFIXES = .moc.cpp .h

# Make call to `moc' emit just `MOC'.
moc_verbose = $(moc_verbose_@AM_V@)
moc_verbose_ = $(moc_verbose_@AM_DEFAULT_V@)
moc_verbose_0 = @echo "  MOC     " $@;

# moc from Qt5 aborts if unknown command line options are supplied;
# in particular, it doesn't recognize `-isystem'
.h.moc.cpp:
	$(moc_verbose)$(MOC) \
	  `echo $(QT_CPPFLAGS) | sed 's/-isystem/-I/g'` \
	  $(EXTRA_CPPFLAGS) \
	  $< -o $@

DISTCLEANFILES = $(BUILT_SOURCES)

AM_CPPFLAGS = -I$(top_srcdir)/lib \
              -I$(top_builddir)/gnulib/src \
              -I$(top_srcdir)/gnulib/src \
              $(FREETYPE_CPPFLAGS)
LDADD = $(top_builddir)/lib/libttfautohint.la \
        $(top_builddir)/lib/libsds.la \
        $(top_builddir)/lib/libnumberset.la \
        $(top_builddir)/gnulib/src/libgnu.la \
        $(LTLIBINTL) \
        $(LTLIBTHREAD) \
        $(FREETYPE_LIBS)

bin_PROGRAMS = ttfautohint
ttfautohint_SOURCES = info.cpp \
                      info.h \
                      main.cpp
manpages = ttfautohint.1

if USE_QT
  bin_PROGRAMS += ttfautohintGUI
  ttfautohintGUI_SOURCES = ddlineedit.cpp \
                           ddlineedit.h \
                           info.cpp \
                           info.h \
                           main.cpp \
                           maingui.cpp \
                           maingui.h \
                           ttlineedit.cpp \
                           ttlineedit.h
  nodist_ttfautohintGUI_SOURCES = ddlineedit.moc.cpp \
                                  maingui.moc.cpp \
                                  static-plugins.cpp \
                                  ttlineedit.moc.cpp

  ttfautohintGUI_CXXFLAGS = $(QT_CXXFLAGS)
  ttfautohintGUI_LDFLAGS = $(QT_LDFLAGS)
  ttfautohintGUI_CPPFLAGS = $(AM_CPPFLAGS) \
                            $(QT_CPPFLAGS) \
                            -DBUILD_GUI
  ttfautohintGUI_LDADD = $(LDADD) \
                         $(QT_LIBS)

  BUILT_SOURCES = ddlineedit.moc.cpp \
                  maingui.moc.cpp \
                  static-plugins.cpp \
                  ttlineedit.moc.cpp

  manpages += ttfautohintGUI.1
endif

if WITH_DOC
  dist_man_MANS = $(manpages)
endif

# `ttfautohint.h' holds default values for some options,
# `ttfautohint-scripts.' the list of available scripts
ttfautohint.1: $(top_srcdir)/frontend/main.cpp \
               $(top_builddir)/lib/ttfautohint.h \
               $(top_srcdir)/lib/ttfautohint-scripts.h \
               $(top_builddir)/.version
	$(MAKE) $(AM_MAKEFLAGS) ttfautohint$(EXEEXT)
	$(HELP2MAN) --output=$@ \
	            --no-info \
	            --name="add new, auto-generated hints to a TrueType font" \
	            ./ttfautohint$(EXEEXT)

ttfautohintGUI.1: $(top_srcdir)/frontend/main.cpp \
                  $(top_builddir)/lib/ttfautohint.h \
                  $(top_srcdir)/lib/ttfautohint-scripts.h \
                  $(top_builddir)/.version
	$(MAKE) $(AM_MAKEFLAGS) ttfautohintGUI$(EXEEXT)
	$(HELP2MAN) --output=$@ \
	            --no-info \
	            --name="add new, auto-generated hints to a TrueType font" \
	            --help-option=--help-all \
	            ./ttfautohintGUI$(EXEEXT)

# end of Makefile.am
