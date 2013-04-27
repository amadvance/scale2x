#!/bin/sh
#

make distclean

if ! ./configure.windows-x86; then
	exit 1
fi

if ! make check distwindows-x86 distclean; then
	exit 1
fi

if ! ./configure ; then
	exit 1
fi

if ! make dist; then
	exit 1
fi

