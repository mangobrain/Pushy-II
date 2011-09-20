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

EXTRA_DIST = pushy.svg pushy.desktop.in

# Sizes (NxN) at which to render and install application icon
SIZES = 16 22 24 32 48 64 72 96 128

# Rule for generating a particular size from original SVG.
# Distribute these so we don't depend on RSVG being
# present on users' systems.  Put the generated files in $(srcdir)
# explicitly, so that we can always rely on them being present in
# $(srcdir) rather than CWD when running install-data-local.
# Writeability of $(srcdir) shouldn't be an issue for end users
# since these files will already exist in an unpacked distribution.
$(SIZES:%=pushy-%.png): pushy-%.png: pushy.svg
	rsvg -w $* -h $* $< $(srcdir)/$@

BUILT_SOURCES = $(SIZES:%=pushy-%.png)
dist_noinst_DATA = $(SIZES:%=pushy-%.png)

# Use xdg-utils to install and uninstall pre-rendered icons.
# I'd love to use it for the scalable icon as well, but
# support for that appears to have been dropped:
# http://lists.freedesktop.org/archives/portland/2006-September/000862.html
# https://bugs.freedesktop.org/show_bug.cgi?id=7837

# Also use xdg-util to install and uninstall .desktop file.

install-data-local: $(SIZES:%=pushy-%.png) pushy.desktop
	for i in $(SIZES); do \
		xdg-icon-resource install --novendor --noupdate --size $$i \
			$(srcdir)/pushy-$$i.png pushy; \
	done
	xdg-icon-resource forceupdate
	xdg-desktop-menu install --novendor pushy.desktop

uninstall-local:
	for i in $(SIZES); do \
		xdg-icon-resource uninstall --noupdate --size $$i pushy; \
	done
	xdg-icon-resource forceupdate
	xdg-desktop-menu uninstall pushy.desktop

# Pre-rendered icons are considered built by the maintainer, because
# they are generated (not handmade) but distributed.  Delete them during
# maintainerclean.
MAINTAINERCLEANFILES = $(SIZES:%=pushy-%.png)

CLEANFILES = pushy.desktop

# Rule for generating .desktop file from .desktop.in.
# This converts the "bindir" placeholder in the ".in" file
# into a fully expanded literal.
pushy.desktop: pushy.desktop.in
	rm -f $@ $@.tmp
	$(SED) -e s'|@bindir[@]|$(bindir)|g' $< >$@.tmp
	mv $@.tmp $@
