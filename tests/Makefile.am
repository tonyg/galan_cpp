## Process this file with automake to produce Makefile.in

EXTRA_PROGRAMS = testref testreg testfns

LDADD = @GLIB_LIBS@
INCLUDES = -I.. -I$(srcdir)/../include @GLIB_CFLAGS@ -DSITE_PKGLIB_DIR=\"$(pkglibdir)\"

testref_SOURCES = testref.cc
testref_LDADD = -liberty

testreg_SOURCES = testreg.cc
testreg_LDADD = ../src/registry.o -lglib
testreg_DEPENDENCIES = ../src/registry.o

testfns_SOURCES = testfns.cc

install: all
all: $(EXTRA_PROGRAMS)

../src/%.o: ../src/%.cc
	make -C ../src %.o

clean:
	rm -f $(EXTRA_PROGRAMS)
