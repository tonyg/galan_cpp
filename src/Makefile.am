## Process this file with automake to produce Makefile.in

bin_PROGRAMS = galan
EXTRA_DIST =

galan_SOURCES = \
	model.cc registry.cc opossum.cc \
	generator.cc macro.cc \
	clock.cc event.cc \
	plugin.cc main.cc \
	msgbox.cc MainWindow.cc

galan_LDFLAGS = -rdynamic `gtkmm-config --libs`
galan_LDADD = @GTK_LIBS@ -ldl

CXXFLAGS = -O0 -g

INCLUDES = \
	-I.. -I$(srcdir)/../include @GTK_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\" \
	`gtkmm-config --cflags`
