#!/bin/sh

CONFIGUREDIR=`dirname $0`
CONFIGURE=${CONFIGUREDIR}/configure

if test "x$1" = "x"; then
	WANTDOC_OPTION=
else
	WANTDOC_OPTION=--with-documentation
fi

VERSION=`grep AM_INIT_AUTOMAKE ../configure.in | sed -e 's/AM_INIT_AUTOMAKE(galan, \(.*\))/\1/'`

APPDIR="`pwd`/QtGalan.app"
CONTENTS=${APPDIR}/Contents

FWKDIR="`pwd`/Galan.framework"
FWKBASE=${FWKDIR}/Versions/${VERSION}

${CONFIGURE} \
    --prefix=${CONTENTS} \
    --bindir=${CONTENTS}/MacOS \
    --with-pkglib-dir=${CONTENTS}/SharedSupport \
    --with-pkgdata-dir=${CONTENTS}/SharedSupport \
    \
    --libdir=${FWKBASE} \
    --includedir=${FWKBASE}/Headers \
    --with-galan-framework=${FWKDIR} \
    \
    ${WANTDOC_OPTION}

