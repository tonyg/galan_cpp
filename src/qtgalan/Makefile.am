## Process this file with automake to produce Makefile.in

MOC=@QTDIR@/bin/moc
UIC=@QTDIR@/bin/uic

bin_PROGRAMS = qtgalan

qtgalan_SOURCES = \
	main.cc \
	mainwin.cc \
	qtio.cc \
	defaultclock.cc \
	macroview.cc \
	regsel.cc \
	regtree.cc \
	itemicon.cc \
	itemlink.cc \
	itemhandle.cc \
	controlpanel.cc \
	helpwindow.cc \
	\
	PluginInfo_base.cc PluginInfo.cc \
	SelectClock_base.cc SelectClock.cc \
	IconLinkEditor_base.cc IconLinkEditor.cc

qtgalan_LDFLAGS = -dlopen self
qtgalan_LDADD = \
	mainwin_moc.o \
	qtio_moc.o \
	defaultclock_moc.o \
	macroview_moc.o \
	regsel_moc.o \
	regtree_moc.o \
	itemicon_moc.o \
	itemhandle_moc.o \
	controlpanel_moc.o \
	helpwindow_moc.o \
	\
	PluginInfo_base_moc.o PluginInfo_moc.o \
	SelectClock_base_moc.o SelectClock_moc.o \
	IconLinkEditor_base_moc.o IconLinkEditor_moc.o \
	\
	../galan/libgalan.la -L@QTDIR@/lib -lqt

noinst_HEADERS = \
	mainwin.h \
	qtio.h \
	defaultclock.h \
	macroview.h \
	regsel.h \
	regtree.h \
	itemicon.h \
	itemlink.h \
	itemhandle.h \
	controlpanel.h \
	helpwindow.h \
	\
	PluginInfo.h \
	SelectClock.h \
	IconLinkEditor.h

BUILT_SOURCES= \
	PluginInfo_base.h \
	SelectClock_base.h \
	IconLinkEditor_base.h

%_base.cc %_base.h: %.ui
	$(UIC) -o $*_base.h $*.ui
	$(UIC) -o $*_base.cc -i $*_base.h $*.ui

# Use this target (by hand) to generate implementation stubs for a new
# .ui file. For example, say you create foo.ui, and want to work with
# foo_base.{cc,h} (which will already be automatically generated for
# you) and foo.{cc,h} (which won't). Then you would:
#
# make foo.impl
#
# ...which would create the stub foo.cc and foo.h for you then to
# customise and fill in. You shouldn't really have to do this more
# than once per .ui file.
#
%.impl: %_base.h
	$(UIC) -o $*.h -subdecl $*Impl $*_base.h $*.ui
	$(UIC) -o $*.cc -subimpl $*Impl $*.h $*.ui

%_moc.cc: %.h
	$(MOC) -o $@ $<

CXXFLAGS = -O0 -g

INCLUDES = \
	-I$(top_srcdir)/src -I@QTDIR@/include @GLIB_CFLAGS@ \
	-DSITE_PKGLIB_DIR=\"$(pkglibdir)\" \
	-DSITE_PKGDATA_DIR=\"$(pkgdatadir)\"

clean distclean:
	rm -f *_moc.cc
	for d in PluginInfo SelectClock ; do rm -f $${d}_base.cc $${d}_base.h ; done
