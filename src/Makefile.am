## Process this file with automake to produce Makefile.in

bin_PROGRAMS = galan
lib_LTLIBRARIES = libgalan.la
EXTRA_DIST =

libgalan_la_SOURCES = \
	model.cc registry.cc \
	generator.cc macro.cc \
	clock.cc event.cc \
	plugin.cc \
	msgbox.cc

libgalan_la_LIBADD = @GLIB_LIBS@
libgalan_la_LDFLAGS = -version-info 0:0:0

galan_SOURCES = main.cc
galan_LDFLAGS = -dlopen self
galan_LDADD = libgalan.la

CXXFLAGS = -O0 -g

INCLUDES = \
	-I.. -I$(srcdir)/../include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"
