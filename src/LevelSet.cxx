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
#include <cstring>

// System

// Library
#include <SDL.h>

// Local
#include "Constants.hxx"
#include "TileSet.hxx"
#include "LevelSet.hxx"

//
// Implementation
//

// Endian-independant read of a 32-bit int from a file
// saved with little-endian ints
uint32_t readInt(std::istream &s)
{
	unsigned char c[4];
	s.read((char*)(&c), 4);
	return (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
}

// Read a NULL terminated string of up to 12 bytes from the file
// Buffer must therefore be at least 13 bytes long
// Chop of trailing carriage return if present
// Optionally reverse each byte value to support level name "decryption"
void readString(std::istream &s, char *buffer, bool decrypt = false)
{
	buffer[12] = '\0';
	s.read(buffer, 12);
	if (decrypt)
	{
		for (int i = 0; i < 12; ++i)
			buffer[i] = 255 - buffer[i];
	}
	char *cr;
	if ((cr = (char*) memchr(buffer, '\r', 12)))
		*cr = '\0';
}

LevelSet::LevelSet(const char *filename)
{
	// Open level set file
	std::ifstream setfile;
	setfile.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
	setfile.open(filename, std::ios_base::binary);

	// Read in number of levels in the set
	uint32_t num_levels = readInt(setfile);
	_levelset.reserve(num_levels);

	// Read numbers of first cross tile and first floor tile
	// Stored as two 32-bit little endian ints, but tile indexes
	// are only one byte, so ignore the bytes we don't need
	setfile.read((char*)(&_first_floor_tile), 1);
	setfile.seekg(3, std::ios::cur);
	setfile.read((char*)(&_first_cross_tile), 1);
	setfile.seekg(3, std::ios::cur);

	// Read in the tile, sprite & player sprite files
	char strbuff[13];
	readString(setfile, strbuff);
	_tileset.reset(new TileSet(strbuff, __TILE_WIDTH, __TILE_HEIGHT));
	readString(setfile, strbuff);
	_spriteset.reset(new TileSet(strbuff, __TILE_WIDTH, __TILE_HEIGHT, true));
	readString(setfile, strbuff);
	_playerspriteset.reset(new TileSet(strbuff, __TILE_WIDTH, __TILE_HEIGHT, true));

	// Read in the title screen tilemap
	setfile.read((char*)_titlescreen, __LEVEL_HEIGHT * __LEVEL_WIDTH);

	// Read in each level
	for (uint32_t i = 0; i < num_levels; ++i)
	{
		// 12 bytes "encrypted" level name
		_levelset.push_back(Level());
		Level &l(_levelset.back());
		readString(setfile, strbuff, true);
		l.name.assign(strbuff);

		// 4 bytes bonus counter start value (BCD)
		setfile.read((char*)(l.bonus), 4);

		// 3 bytes (ignore 4th) of level name colour
		setfile.read((char*)(l.name_colour), 3);
		setfile.seekg(1, std::ios::cur);

		// tile map
		setfile.read((char*)(l.tilemap), __LEVEL_HEIGHT * __LEVEL_WIDTH);

		// number of sprites
		l.num_sprites = readInt(setfile);
		if (l.num_sprites > __MAX_SPRITES_PER_LEVEL)
			throw std::runtime_error("Too many sprites in level");
		
		// Read in sprite info
		for (uint32_t j = 0; j < l.num_sprites; ++j)
		{
			setfile.read((char*)&(l.spriteinfo[j].x), 1);
			setfile.read((char*)&(l.spriteinfo[j].y), 1);
			setfile.read((char*)&(l.spriteinfo[j].index), 1);
		}

		// Skip junk data if we don't have a full sprite info section
		setfile.seekg(
			(__MAX_SPRITES_PER_LEVEL - l.num_sprites) * 3,
			std::ios::cur
		);
	}
}
