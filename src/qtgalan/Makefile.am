## Process this file with automake to produce Makefile.in

bin_PROGRAMS = qtgalan

qtgalan_SOURCES = main.cc
qtgalan_LDFLAGS = -dlopen self
qtgalan_LDADD = ../galan/libgalan.la -L@QTDIR@/lib -lqt

CXXFLAGS = -O0 -g

INCLUDES = \
	-I$(top_srcdir)/src -I@QTDIR@/include @GLIB_CFLAGS@
