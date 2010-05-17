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

// System

// Library
#include <SDL.h>

// Local
#include "Constants.hxx"
#include "TileSet.hxx"
#include "GameObjects.hxx"

//
// Implementation
//

GameObject::GameObject(TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject *objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	: _sprites(sprites), _x(x), _y(y), _objects(objects),
	_tilemap(tilemap), _first_floor_tile(first_floor_tile),
	_first_cross_tile(first_cross_tile)
{
}

bool GameObject::tileIsEmpty(uint8_t x, uint8_t y) const
{
	// Floor and cross tiles can be moved into
	return (_tilemap[(y * __LEVEL_WIDTH) + x] >= _first_floor_tile);
}

bool GameObject::tileIsCross(uint8_t x, uint8_t y) const
{
	return (_tilemap[(y * __LEVEL_WIDTH) + x] >= _first_cross_tile);
}

bool PushableObject::canMove(Direction d) const
{
	// TODO Check _gameobjects for emptiness too
	switch (d)
	{
		case Up:
			if (_y == 0)
				return false;
			return tileIsEmpty(_x, _y - 1);
		case Down:
			if (_y == __LEVEL_HEIGHT)
				return false;
			return tileIsEmpty(_x, _y + 1);
		case Left:
			if (_x == 0)
				return false;
			return tileIsEmpty(_x - 1, _y);
		case Right:
			if (_x == __LEVEL_WIDTH)
				return false;
			return tileIsEmpty(_x + 1, _y);
	}
}
