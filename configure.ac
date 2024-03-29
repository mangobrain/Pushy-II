# Copyright 2010-2011 Philip Allison

#    This file is part of Pushy 2.
#
#    Pushy 2 is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Pushy 2 is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Pushy 2.  If not, see <http://www.gnu.org/licenses/>.

dnl # Specify autoconf version requirements
dnl # Not strictly necessary - but it's the version on my system,
dnl # so for now, we'll pretend it's the required version.
AC_PREREQ([2.63])

dnl # Initialise autoconf
AC_INIT([Pushy 2], [0.1], [http://pushy.mangobrain.co.uk/])
AC_CONFIG_SRCDIR([src/main.cxx])
AC_CONFIG_AUX_DIR([build-aux])

dnl # Initialis automake - first version with silent building support
AM_INIT_AUTOMAKE([1.11])
AM_SILENT_RULES([yes])

dnl # Pushy 2 is written in C++.  What else is there?
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

dnl # Make config.h from config.h.in.  (`autoheader' generates the latter.)
AC_CONFIG_HEADERS([config.h])

dnl # Output the command-line options given to this script
AC_DEFINE_UNQUOTED(
	[P2_CONFIGURE_OPTS],
	["$ac_configure_args"],
	[Options given to ./configure]
)

dnl # Use pkg-config to find various dependencies
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([SDL], [sdl >= 1.2])

dnl # We use getopt_long for parsing our command-line options
AC_CHECK_HEADERS_ONCE([getopt.h])
AS_IF(
	[test "x$ac_cv_header_getopt_h" != "xyes"],
	[AC_MSG_ERROR([We need getopt.h for option parsing!])]
)

dnl # Installation of icons and a .desktop file is optional,
dnl # because technically it might involve installing files
dnl # outside the configured installation prefix.
AC_ARG_WITH(
	[freedesktop],
	[AS_HELP_STRING([--with-freedesktop], [Install desktop menu entry.  This is disabled by default, as it uses xdg-utils, which may install files outside PREFIX.])],
	[
		AS_IF(
			[test "x$withval" = "xyes"],
			[xdg=true],
			[xdg=false]
		)
	],
	[xdg=false]
)
AM_CONDITIONAL([XDG], [test "x$xdg" = "xtrue"])
AS_IF(
	[test "x$xdg" = "xtrue"],
	[
		dnl # Sed is used to parse pushy.desktop.in
		AC_PROG_SED

		dnl # xdg-utils are used to install icons and .desktop file
		AC_PATH_PROG([XDG_IR], [xdg-icon-resource], [notfound])
		AC_PATH_PROG([XDG_DM], [xdg-desktop-menu], [notfound])

		AS_IF(
			[test "x$XDG_IR" = "xnotfound"],
			[AC_MSG_ERROR([Could not find xdg-icon-resource in PATH])]
		)

		AS_IF(
			[test "x$XDG_DM" = "xnotfound"],
			[AC_MSG_ERROR([Could not find xdg-desktop-menu in PATH])]
		)
	]
)

dnl # All done.  Spit out the result.
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	data/Makefile
	icons/Makefile
])
AC_OUTPUT
