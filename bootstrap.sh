#!/bin/sh
aclocal -I m4 --force
autoheader
automake --add-missing --copy --force-missing --foreign
autoconf
