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

//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language
#include <fstream>
#include <stdexcept>
#include <string>
#include <new>
#include <memory>

// System

// Library
#include <SDL.h>

// Local
#include "TileSet.hxx"

//
// Implementation
//

TileSet::TileSet(const char *filename, int width, int height)
{
	// Calculate size in tileset file of a single tile
	// Input files are raw 32-bit bitmaps
	std::ifstream::pos_type tilesize = width * height * 4;

	// Open tile set file - at the end, so we can determine file size
	std::ifstream setfile;
	setfile.exceptions(std::ios::badbit | std::ios::failbit);
	setfile.open(filename, std::ios_base::binary | std::ios_base::ate);
	std::ifstream::pos_type filesize = setfile.tellg();

	// Seek back to the beginning and read in images one by one
	setfile.seekg(0);
	while (filesize - setfile.tellg() >= tilesize)
	{
		// Allocate buffer and read in raw data
		std::auto_ptr<char> buff(new char[tilesize]);
		setfile.read(buff.get(), tilesize);
		unsigned char *ubuff = (unsigned char*)(buff.get());

		// Allocate a new SDL_Surface to hold the data
		SDL_Surface *tile = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 24,
			0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
		if (!tile)
		{
			for (std::vector<SDL_Surface*>::iterator i = _tiles.begin();
				i < _tiles.end(); ++i)
			{
				SDL_FreeSurface(*i);
			}
			throw std::runtime_error(
				std::string("Cannot allocate SDL surface for tile: ")
				.append(SDL_GetError())
			);
		}

		// Convert the data to the surface's pixel format and write it out
		SDL_FillRect(tile, NULL, SDL_MapRGB(tile->format, 0x00, 0x00, 0x00));
		for (int y = 0; y < height; ++y)
		{
			size_t rowstart = y * tile->pitch;
			size_t xoff = 0;
			for (int x = 0; x < width; ++x)
			{
				uint32_t *pixel = (uint32_t*)(((char*)tile->pixels) + rowstart + xoff);
				size_t boff = (y * width * 4) + (x * 4);
				*pixel |= ((ubuff[boff] >> tile->format->Rloss) << tile->format->Rshift)
					| ((ubuff[boff + 1] >> tile->format->Gloss) << tile->format->Gshift)
					| ((ubuff[boff + 2] >> tile->format->Bloss) << tile->format->Bshift);
				xoff += tile->format->BytesPerPixel;
			}
		}

		_tiles.push_back(tile);
	}
}

TileSet::~TileSet()
{
	for (std::vector<SDL_Surface*>::iterator i = _tiles.begin(); i < _tiles.end(); ++i)
	{
		SDL_FreeSurface(*i);
	}
}
