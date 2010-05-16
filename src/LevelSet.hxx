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

#ifndef __HXX_LEVELSET
#define __HXX_LEVELSET

#include <vector>
#include <memory>
#include <string>
#include <cstddef>

// Structure representing one sprite in a level
struct SpriteInfo
{
	uint8_t x;
	uint8_t y;
	uint8_t index;
};

// Structure representing one level from a set
struct Level
{
	std::string name;
	uint8_t bonus[4];
	unsigned char name_colour[3];
	uint8_t num_sprites;
	SpriteInfo spriteinfo[__MAX_SPRITES_PER_LEVEL];
	uint8_t tilemap[__LEVEL_HEIGHT * __LEVEL_WIDTH];
};

// Load in a level set, including the tiles and sprites it requires
// Every level in the set is assumed to be 20*12 tiles in size
class LevelSet
{
	public:
		LevelSet(const char *filename);

		const Level &operator[](int index) const
		{
			return _levelset[index];
		};

		std::vector<Level>::size_type size() const
		{
			return _levelset.size();
		};

		const TileSet &getTiles() const
		{
			return *_tileset;
		};

		const TileSet &getSprites() const
		{
			return *_spriteset;
		};

		const TileSet &getPlayerSprites() const
		{
			return *_playerspriteset;
		};

		const uint8_t *getTitleScreen() const
		{
			return _titlescreen;
		};
		
		uint8_t firstFloorTile() const
		{
			return _first_floor_tile;
		};
		
		uint8_t firstCrossTile() const
		{
			return _first_cross_tile;
		};

	private:
		std::auto_ptr<TileSet> _tileset;
		std::auto_ptr<TileSet> _spriteset;
		std::auto_ptr<TileSet> _playerspriteset;
		std::vector<Level> _levelset;
		uint8_t _titlescreen[__LEVEL_HEIGHT * __LEVEL_WIDTH];
		uint8_t _first_floor_tile;
		uint8_t _first_cross_tile;
};

#endif
