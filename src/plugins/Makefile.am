## Process this file with automake to produce Makefile.in

#EXTRA_DIST = TEMPLATE.cc

plugindir = $(pkglibdir)/plugins

# the configure variable OPTIONAL_PLUGINS consists of a (possibly
# empty) whitespace-separated list of names from the list just
# below. (The optional plugins HAVE to be defined in EXTRA_PROGRAMS -
# see the infopages for automake.)
EXTRA_PROGRAMS =

plugin_PROGRAMS = \
	Test.la \
	OssOutput.la \
	VCO.la \
	Gain.la \
	Delay.la \
	Vorbis.la \
	Controllers.la \
	@OPTIONAL_PLUGINS@

#CXXFLAGS = -O0 -g
CXXFLAGS = -O3 -g

LDFLAGS = -module -rpath $(plugindir) -avoid-version -export-symbols-regex init_plugin_.*
#LIBADD = ../src/libgalan.la
INCLUDES = -I$(top_srcdir)/src @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"

#esd_output_so_INCLUDES=`esd-config --cflags`
#esd_output_so_LDFLAGS=`esd-config --libs`

Test_la_SOURCES = Test.cc
OssOutput_la_SOURCES = OssOutput.cc
VCO_la_SOURCES = VCO.cc
Gain_la_SOURCES = Gain.cc
Delay_la_SOURCES = Delay.cc

Vorbis_la_SOURCES = Vorbis.cc
Vorbis_la_CFLAGS = $(VORBIS_CFLAGS)
Vorbis_la_LDADD = $(VORBISFILE_LIBS) $(VORBIS_LIBS)

Controllers_la_SOURCES = Controllers.cc
