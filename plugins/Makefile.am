## Process this file with automake to produce Makefile.in

#EXTRA_DIST = TEMPLATE.cc

plugindir = $(pkglibdir)/plugins

# the configure variable OPTIONAL_PLUGINS consists of a (possibly
# empty) whitespace-separated list of names from the list just
# below. (The optional plugins HAVE to be defined in EXTRA_PROGRAMS -
# see the infopages for automake.)
EXTRA_PROGRAMS =

plugin_PROGRAMS = \
	Test.so \
	@OPTIONAL_PLUGINS@

#	OssOutput.so

CXXFLAGS = -O0 -g
LDFLAGS = -shared
LDADD = @GLIB_LIBS@
INCLUDES = -I.. -I$(srcdir)/../include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\" -DPIC -fPIC

#esd_output_so_INCLUDES=`esd-config --cflags`
#esd_output_so_LDFLAGS=`esd-config --libs`

Test_so_SOURCES = Test.cc
#OssOutput_so_SOURCES = OssOutput.cc
