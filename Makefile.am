## Process this file with automake to produce Makefile.in

EXTRA_DIST = doc galan.spec TODO
SUBDIRS = src tests

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

tags:
	etags `find . -name "*.h" -o -name "*.c" -o -name "*.cc"`

clean:
	make -C doc clean
