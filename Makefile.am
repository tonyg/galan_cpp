## Process this file with automake to produce Makefile.in

EXTRA_DIST = doc galan.spec TODO MANIFEST

if WantDocumentation
SUBDIRS = src tests doc
else
SUBDIRS = src tests
endif

bin_SCRIPTS = galan-config

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

tags:
	etags `find . -name "*.h" -o -name "*.c" -o -name "*.cc"`

clean:
	make -C doc clean

if NoMacOSXFramework
else
install-exec-local:
	ln -sf @FRAMEWORK_DIRECTORY@/Versions/@VERSION@ @FRAMEWORK_DIRECTORY@/Versions/Current
	ln -sf @FRAMEWORK_DIRECTORY@/Versions/Current/* @FRAMEWORK_DIRECTORY@
	test -d $(prefix)/Resources || mkdir -p $(prefix)/Resources
	cp $(srcdir)/doc/qtgalan.icns $(prefix)/Resources
	cp doc/Info.plist $(prefix)
	( \
		LIBNAME=`otool -L $(bindir)/qtgalan | grep libqt | cut -d' ' -f 1 | tr -d ' \t'` \
		&& (test -d $(prefix)/Frameworks || mkdir -p $(prefix)/Frameworks) \
		&& cp $(QTDIR)/lib/$$LIBNAME $(prefix)/Frameworks \
		&& install_name_tool -id "@executable_path/../Frameworks/$$LIBNAME" \
			$(prefix)/Frameworks/$$LIBNAME \
		&& install_name_tool -change $$LIBNAME "@executable_path/../Frameworks/$$LIBNAME" \
			$(bindir)/qtgalan \
	)
endif
