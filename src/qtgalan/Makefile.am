## Process this file with automake to produce Makefile.in

bin_PROGRAMS = qtgalan

qtgalan_SOURCES = \
	main.cc \
	mainwin.cc \
	qtio.cc \
	popupdialog.cc \
	defaultclock.cc

qtgalan_LDFLAGS = -dlopen self
qtgalan_LDADD = \
	mainwin_moc.o \
	qtio_moc.o \
	popupdialog_moc.o \
	defaultclock_moc.o \
	../galan/libgalan.la -L@QTDIR@/lib -lqt

noinst_HEADERS = \
	mainwin.h \
	qtio.h \
	popupdialog.h \
	defaultclock.h

%_moc.cc: %.h
	@QTDIR@/bin/moc $< > $@

CXXFLAGS = -O0 -g

INCLUDES = \
	-I$(top_srcdir)/src -I@QTDIR@/include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"
