// Copyright 2010 Philip Allison

//    This file is part of Pushy 2.
//
//    Pushy 2 is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Pushy 2 is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Pushy 2.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __HXX_TILESET
#define __HXX_TILESET

#include <vector>

// A collection of equal-sized, 24 bpp SDL surfaces loaded from a
// file containing concatenated raw bitmap data
class TileSet
{
	public:
		TileSet(const char *filename, int width, int height, bool colorkey = false);
		~TileSet();

		SDL_Surface *operator[](std::vector<SDL_Surface*>::size_type index) const
		{
			return _tiles[index];
		};

		std::vector<SDL_Surface*>::size_type size() const
		{
			return _tiles.size();
		};
	private:
		std::vector<SDL_Surface*> _tiles;
};

#endif
