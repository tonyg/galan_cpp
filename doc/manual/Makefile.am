pkgdatadir = @PKGDATA_DIRECTORY@
manualdir = $(pkgdatadir)/manual

figures =

manual_DATA = \
	galan-style.css \
	\
	$(wildcard *-*.html) \
	$(figures) \
	\
	index.html \
	userguide.html

EXTRA_DIST = \
	galandoc.dtd.in \
	galan-style.css \
	webdoc.dsl \
	xform-classes.dsl \
	$(figures) \
	\
	index.xml \
	userguide.xml

%.html: %.xml webdoc.dsl xform-classes.dsl galandoc.dtd
	-cp galandoc.dtd $(srcdir)
	openjade -V '%base-filename%=$*' -t xml $<

clean:
	rm -f *.html
