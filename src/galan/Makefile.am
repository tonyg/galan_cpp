## Process this file with automake to produce Makefile.in

bin_PROGRAMS = galan
lib_LTLIBRARIES = libgalan.la
EXTRA_DIST =

pkginclude_HEADERS = \
	ref.h model.h registry.h \
	sample.h generator.h macro.h \
	clock.h event.h \
	iomanager.h \
	plugin.h \
	msgbox.h

libgalan_la_SOURCES = \
	model.cc registry.cc \
	generator.cc macro.cc \
	clock.cc event.cc \
	iomanager.cc \
	plugin.cc \
	msgbox.cc

libgalan_la_LIBADD = @GLIB_LIBS@
libgalan_la_LDFLAGS = -version-info 0:0:0

galan_SOURCES = main.cc
galan_LDFLAGS = -dlopen self
galan_LDADD = libgalan.la

CXXFLAGS = -O0 -g

INCLUDES = \
	-I$(top_srcdir)/src @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"
