## Process this file with automake to produce Makefile.in

bin_PROGRAMS = galan
EXTRA_DIST =

galan_SOURCES = \
	model.cc registry.cc \
	generator.cc macro.cc \
	clock.cc event.cc \
	plugin.cc main.cc \
	msgbox.cc

galan_LDFLAGS = -rdynamic
galan_LDADD = @GLIB_LIBS@ -ldl

CXXFLAGS = -O0 -g

INCLUDES = \
	-I.. -I$(srcdir)/../include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"
