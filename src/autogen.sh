#!/bin/sh -e

aclocal
automake --add-missing --copy
autoconf
