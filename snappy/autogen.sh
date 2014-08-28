#! /bin/sh -e
rm -rf autom4te.cache
aclocal -I m4
autoheader
glibtoolize --copy
automake --add-missing --copy
autoconf
