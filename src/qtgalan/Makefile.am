## Process this file with automake to produce Makefile.in

MOC=@QTDIR@/bin/moc
UIC=@QTDIR@/bin/uic

bin_PROGRAMS = qtgalan

qtgalan_SOURCES = \
	main.cc \
	mainwin.cc \
	qtio.cc \
	defaultclock.cc \
	\
	PluginInfo_base.cc PluginInfo.cc \
	SelectClock_base.cc SelectClock.cc

qtgalan_LDFLAGS = -dlopen self
qtgalan_LDADD = \
	mainwin_moc.o \
	qtio_moc.o \
	defaultclock_moc.o \
	\
	PluginInfo_base_moc.o PluginInfo_moc.o \
	SelectClock_base_moc.o SelectClock_moc.o \
	\
	../galan/libgalan.la -L@QTDIR@/lib -lqt

noinst_HEADERS = \
	mainwin.h \
	qtio.h \
	defaultclock.h \
	\
	PluginInfo.h \
	SelectClock.h

BUILT_SOURCES= \
	PluginInfo_base.h \
	SelectClock_base.h

%_base.cc %_base.h: %.ui
	$(UIC) -o $*_base.h $*.ui
	$(UIC) -o $*_base.cc -i $*_base.h $*.ui

# Use this target to generate implementation stubs for a new .ui file.
%.impl: %_base.h
	$(UIC) -o $*.h -subdecl $*Impl $*_base.h $*.ui
	$(UIC) -o $*.cc -subimpl $*Impl $*.h $*.ui

%_moc.cc: %.h
	$(MOC) -o $@ $<

CXXFLAGS = -O0 -g

INCLUDES = \
	-I$(top_srcdir)/src -I@QTDIR@/include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"

clean:
	rm -f *_moc.cc
	for d in PluginInfo SelectClock ; do rm -f $${d}_base.cc $${d}_base.h ; done
