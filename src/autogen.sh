#!/bin/sh -e

aclocal -I m4
automake --add-missing --copy
autoconf
