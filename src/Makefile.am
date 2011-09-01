# Copyright 2010 Philip Allison

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

bin_PROGRAMS = pushy2

pushy2_SOURCES = main.cxx TileSet.hxx TileSet.cxx LevelSet.hxx LevelSet.cxx \
	GameObjects.hxx GameObjects.cxx Constants.hxx Alphabet.hxx Alphabet.cxx \
	GameLoop.hxx InGame.hxx InGame.cxx MainMenu.hxx MainMenu.cxx
pushy2_CXXFLAGS = $(SDL_CFLAGS) $(AM_CXXFLAGS)
pushy2_CPPFLAGS = -DP2_PKGDATADIR='"$(pkgdatadir)"' $(AM_CPPFLAGS)
pushy2_LDFLAGS = $(SDL_LIBS) $(AM_LDFLAGS)
