## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libgalan.la

pkglibdir = @PKGLIB_DIRECTORY@

pkginclude_HEADERS = \
	global.h \
	model.h registry.h \
	sample.h generator.h macro.h \
	clock.h event.h \
	iomanager.h \
	controller.h \
	plugin.h

libgalan_la_SOURCES = \
	model.cc registry.cc \
	generator.cc macro.cc \
	clock.cc event.cc \
	iomanager.cc \
	controller.cc \
	plugin.cc

libgalan_la_LIBADD = @GLIB_LIBS@
libgalan_la_LDFLAGS = -version-info 0:0:0

#CXXFLAGS = -O0 -g
#CXXFLAGS = -O3 -g
CXXFLAGS = -O3 -g -pg
#CXXFLAGS = -O0 -g -pg

INCLUDES = \
	-I$(top_srcdir)/src @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"
