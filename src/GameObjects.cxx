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

#define PLAYER_SPEED 4
#define PUSH_SPEED 2
#define ROLL_SPEED 4

extern int objects_left;

GameObject::GameObject(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
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
	return (_tilemap[(y * __LEVEL_WIDTH) + x] < _first_cross_tile);
}

bool PushableObject::canMove(Direction d) const
{
	uint8_t cx = _x;
	uint8_t cy = _y;
	switch (d)
	{
		case Up:
			if (cy == 0)
				return false;
			--cy;
			break;
		case Down:
			if (cy == __LEVEL_HEIGHT)
				return false;
			++cy;
			break;
		case Left:
			if (cx == 0)
				return false;
			--cx;
			break;
		case Right:
			if (cx == __LEVEL_WIDTH)
				return false;
			++cx;
	}
	return (tileIsEmpty(cx, cy) && !_objects[(cy * __LEVEL_WIDTH) + cx]);
}

PushableObject::PushableObject(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:GameObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile),
	AnimableObject(x, y), _defused(false)
{}

AnimableObject::AnimableObject(uint8_t ax, uint8_t ay)
	:_anim_x(ax * __TILE_WIDTH), _anim_y(ay * __TILE_HEIGHT),
	_anim_index(0), _anim_state(0)
{}

Ball::Ball(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:PushableObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile),
	_rolling(false), _speed(PUSH_SPEED), _roll_momentum(0)
{}

Box::Box(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:PushableObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile)
{}

Player::Player(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:GameObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile),
	AnimableObject(x, y), _speed(PLAYER_SPEED), _busy(false), _push_momentum(0), _straining(false)
{}

bool AnimableObject::arrived(uint8_t x, uint8_t y) const
{
	return ((_anim_x == (uint32_t)x * __TILE_WIDTH) && (_anim_y == (uint32_t)y * __TILE_HEIGHT));
}

void AnimableObject::slideTo(uint8_t x, uint8_t y, uint32_t speed)
{
	if (_anim_x > (uint32_t)x * __TILE_WIDTH)
		_anim_x -= speed;
	else if (_anim_x < (uint32_t)x * __TILE_WIDTH)
		_anim_x += speed;
	else if (_anim_y > (uint32_t)y * __TILE_HEIGHT)
		_anim_y -= speed;
	else if (_anim_y < (uint32_t)y * __TILE_HEIGHT)
		_anim_y += speed;
}

bool Ball::rolls() const
{
	return true;
}

void Ball::push(Direction d)
{
	// Immediately move to the furthest empty square in the given direction
	// Move until we hit a wall or filled square, then back up one
	// Be careful not to go off the edges
	int cx = _x;
	int cy = _y;
	switch (d)
	{
		case Up:
			for (cy = _y - 1; cy > 0; --cy)
			{
				if (!tileIsEmpty(_x, cy) || _objects[(cy * __LEVEL_WIDTH) + _x])
				{
					++cy;
					break;
				}
			}
			break;
		case Down:
			for (cy = _y + 1; cy < __LEVEL_HEIGHT; ++cy)
			{
				if (!tileIsEmpty(_x, cy) || _objects[(cy * __LEVEL_WIDTH) + _x])
				{
					--cy;
					break;
				}
			}
			break;
		case Left:
			for (cx = _x - 1; cx > 0; --cx)
			{
				if (!tileIsEmpty(cx, _y) || _objects[(_y * __LEVEL_WIDTH) + cx])
				{
					++cx;
					break;
				}
			}
			break;
		case Right:
			for (cx = _x + 1; cx < __LEVEL_WIDTH; ++cx)
			{
				if (!tileIsEmpty(cx, _y) || _objects[(_y * __LEVEL_WIDTH) + cx])
				{
					--cx;
					break;
				}
			}
	}
	_objects[(_y * __LEVEL_WIDTH) + _x] = NULL;
	_objects[(cy * __LEVEL_WIDTH) + cx] = this;
	_x = cx;
	_y = cy;
	_rolling = true;
	_speed = PUSH_SPEED;
	_roll_momentum = 0;
}

bool Box::rolls() const
{
	return false;
}

void Box::push(Direction d)
{
	// Move one square in given direction
	int cx = _x;
	int cy = _y;
	switch (d)
	{
		case Up:
			--cy;
			break;
		case Down:
			++cy;
			break;
		case Left:
			--cx;
			break;
		case Right:
			++cx;
	}
	_objects[(_y * __LEVEL_WIDTH) + _x] = NULL;
	_objects[(cy * __LEVEL_WIDTH) + cx] = this;
	_x = cx;
	_y = cy;
}

void Ball::render(SDL_Surface *screen)
{
	if (_rolling)
	{
		slideTo(_x, _y, _speed);
		if (arrived(_x, _y))
			_rolling = false;
		// First half of first square is slid to at
		// half normal speed, the momentum picks up
		if ((_roll_momentum += _speed) >= __TILE_WIDTH / 2)
			_speed = ROLL_SPEED;
	}
	// Check (when arrived) whether destination is a cross, and defuse
	if (!_rolling && !_defused && tileIsCross(_x, _y))
	{
		--objects_left;
		_defused = true;
	}
	if (_defused && !tileIsCross(_x, _y))
	{
		// We've been pushed off a cross, arrived or otherwise
		++objects_left;
		_defused = false;
	}
	// 8-14 = on fire, 15-19 = defused
	// minus 8, that makes 0-6 = on fire, 7-11 defused
	// double up frame indexes to slow down animation rate
	switch (_anim_state)
	{
		case 0:
			// on fire - animation looping upwards
			if (++_anim_index == 13)
			{
				if (_defused)
					_anim_state = 2;
				else
				{
					_anim_state = 1;
					// Don't stay at loop end frame
					// for three frames in a row
					--_anim_index;
				}
			}
			break;
		case 1:
			// on fire - animation looping downwards
			if (--_anim_index == 0)
			{
				_anim_state = 0;
				// Don't stay at loop end frame
				// for three frames in a row
				++_anim_index;
			}
			break;
		case 2:
			// defused - looping upwards
			if (++_anim_index == 23)
			{
				_anim_state = 3;
				// Don't stay at loop end frame
				// for three frames in a row
				--_anim_index;
			}
			break;
		case 3:
			// defused - looping downwards
			if (--_anim_index == 14)
			{
				if (_defused)
				{
					_anim_state = 2;
					// Don't stay at loop end frame
					// for three frames in a row
					++_anim_index;
				}
				else
					_anim_state = 1;
			}
	}
	SDL_Rect rect = {
		_anim_x, _anim_y,
		0, 0
	};
	// frame indexes are doubled to slow down animation rates
	// halve them before accessing the sprite array
	SDL_Surface *sprite = (*_sprites)[(_anim_index / 2) + 8];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
}

void Box::render(SDL_Surface *screen)
{
	slideTo(_x, _y, PUSH_SPEED);
	if (!_defused && tileIsCross(_x, _y) && arrived(_x, _y))
	{
		// We've arived on a cross
		--objects_left;
		_defused = true;
	}

	if (_defused && !tileIsCross(_x, _y))
	{
		// We've been pushed off a cross, arrived or otherwise
		++objects_left;
		_defused = false;
	}

	SDL_Rect rect = {
		_anim_x, _anim_y,
		0, 0
	};
	// animation frame indexes:
	// 0-6 = on fire, 7 = defused
	switch (_anim_state)
	{
		case 0:
			// on fire - animation looping forwards
			// double up frame indexes to slow down animation rate
			if (++_anim_index == 13)
			{
				if (_defused)
					_anim_state = 2;
				else
				{
					_anim_state = 1;
					// Don't stay at loop end frame
					// for three frames in a row
					--_anim_index;
				}
			}
			break;
		case 1:
			// on fire - animation looping backwards
			if (--_anim_index == 0)
			{
				_anim_state = 0;
				// Don't stay at loop end frame
				// for three frames in a row
				++_anim_index;
			}
			break;
		case 2:
			// defused
			_anim_index = 14;
			if (!_defused)
			{
				--_anim_index;
				_anim_state = 1;
			}
	}
	// frame indexes are doubled to slow down animation rates
	// halve them before accessing the sprite array
	SDL_Surface *sprite = (*_sprites)[_anim_index / 2];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
}

void Player::render(SDL_Surface *screen)
{
	if (_busy)
	{
		slideTo(_x, _y, _speed);

		if (arrived(_x, _y))
		{
			// We've arrived - we have finished pushing for now
			_speed = PLAYER_SPEED;
			_busy = false;
		}

		// If we're pushing a rolling object, eventually
		// it picks up speed and we can move more freely
		if (_push_momentum > 0)
		{
			if ((_push_momentum -= PUSH_SPEED) == 0)
			{
				_speed = PLAYER_SPEED;
				_straining = false;
			}
		}
	}
	// Animate up/down/left/right loops
	// All are 6 frames long, but go too quickly at 30fps,
	// so count to 12 and only step every other frame
	if (++_anim_index == 12)
		_anim_index = 0;
	SDL_Rect rect = {
		_anim_x, _anim_y,
		0, 0
	};
	SDL_Surface *sprite = (*_sprites)[(_anim_index / 2) + _anim_state + (_straining ? 24 : 0)];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
	// If player pushed against a wall, only stay in the left/right/up/down
	// animation for one frame, unless they keep the key held down
	if (!_busy)
	{
		_anim_state = 0;
		_straining = false;
	}
}

void Player::move(Direction d)
{
	// Can't change direction whilst moving
	if (_busy)
		return;

	int cx = _x;
	int cy = _y;
	switch (d)
	{
		case Up:
			if (cy > 0)
				--cy;
			// First frame of up animation
			_anim_state = 6;
			break;
		case Down:
			if (cy < __LEVEL_HEIGHT)
				++cy;
			// First frame of down animation
			_anim_state = 12;
			break;
		case Left:
			if (cx > 0)
				--cx;
			// First frame of left animation
			_anim_state = 18;
			break;
		case Right:
			if (cx < __LEVEL_WIDTH)
				++cx;
			// First frame of right animation
			_anim_state = 24;
	}
	// Are we away from the edges?
	if (cx != _x || cy != _y)
	{
		// Is the space blocked by a wall?
		if (tileIsEmpty(cx, cy))
		{
			// Is there a pushable object there?
			GameObject *o = _objects[(cy * __LEVEL_WIDTH) + cx];
			if (o)
			{
				// Use strain animations up against objects,
				// movable or otherwise
				_straining = true;
				if (((PushableObject*)o)->canMove(d))
				{
					((PushableObject*)o)->push(d);
					// We're straining - reduce movement speed
					_speed = PUSH_SPEED;
					// But maybe only for half a square
					if (((PushableObject*)o)->rolls())
						_push_momentum = __TILE_WIDTH / 2;
				}
				else
					return;
			}
			// We can move
			_objects[(_y * __LEVEL_WIDTH) + _x] = NULL;
			_objects[(cy * __LEVEL_WIDTH) + cx] = this;
			_x = cx;
			_y = cy;
			_busy = true;
		}
		else
		{
			// Use strain animations up against walls
			_straining = true;
		}
	}
}
