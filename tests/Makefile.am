## Process this file with automake to produce Makefile.in

EXTRA_PROGRAMS = testreg testfns

LDADD = @GLIB_LIBS@
INCLUDES = -I$(top_srcdir)/src @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"

testreg_SOURCES = testreg.cc
testreg_LDADD = ../src/galan/registry.o ../src/galan/model.o -lglib
testreg_DEPENDENCIES = ../src/galan/registry.o ../src/galan/model.o

testfns_SOURCES = testfns.cc

install: all
all: $(EXTRA_PROGRAMS)

../src/galan/%.o: ../src/galan/%.cc
	make -C ../src %.o

clean:
	rm -f $(EXTRA_PROGRAMS)
