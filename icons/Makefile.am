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

EXTRA_DIST = pushy.svg pushy.desktop

dist-hook: $(SIZES:%=pushy-%.png)
	cp -p pushy-*.png $(distdir)/

# Sizes (NxN) at which to render and install application icon
SIZES = 16 22 24 32 48 64 72 96 128

# Rule for generating a particular size from original SVG.
# Also do this at distribution time so we don't depend on RSVG being
# present on users' systems.
pushy-%.png: pushy.svg
	rsvg -w $* -h $* $< $@

# Use xdg-utils to install and uninstall pre-rendered icons.
# I'd love to use it for the scalable icon as well, but
# support for that appears to have been dropped:
# http://lists.freedesktop.org/archives/portland/2006-September/000862.html
# https://bugs.freedesktop.org/show_bug.cgi?id=7837

# Also use xdg-util to install and uninstall .desktop file.

install-data-local: $(SIZES:%=pushy-%.png) pushy.desktop
	for i in $(SIZES); do \
		xdg-icon-resource install --novendor --noupdate --size $$i \
			pushy-$$i.png pushy; \
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
maintainer-clean-local:
	rm -f pushy-*.png

# Rule for generating .desktop file from .desktop.in.
# This converts the "bindir" placeholder in the ".in" file
# into a fully expanded literal.
pushy.desktop: pushy.desktop.in
	rm -f $@ $@.tmp
	$(SED) -e s'|@bindir[@]|$(bindir)|g' $< >$@.tmp
	mv $@.tmp $@
