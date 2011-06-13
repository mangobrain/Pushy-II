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
	: m_sprites(sprites), m_x(x), m_y(y), m_objects(objects),
	m_tilemap(tilemap), m_first_floor_tile(first_floor_tile),
	m_first_cross_tile(first_cross_tile)
{
}

bool GameObject::tileIsEmpty(uint8_t x, uint8_t y) const
{
	// Floor and cross tiles can be moved into
	return (m_tilemap[(y * P2_LEVEL_WIDTH) + x] >= m_first_floor_tile);
}

bool GameObject::tileIsCross(uint8_t x, uint8_t y) const
{
	return (m_tilemap[(y * P2_LEVEL_WIDTH) + x] < m_first_cross_tile);
}

bool PushableObject::canMove(Direction d) const
{
	uint8_t cx = m_x;
	uint8_t cy = m_y;
	switch (d)
	{
		case Up:
			if (cy == 0)
				return false;
			--cy;
			break;
		case Down:
			if (cy == P2_LEVEL_HEIGHT)
				return false;
			++cy;
			break;
		case Left:
			if (cx == 0)
				return false;
			--cx;
			break;
		case Right:
			if (cx == P2_LEVEL_WIDTH)
				return false;
			++cx;
	}
	return (tileIsEmpty(cx, cy) && !m_objects[(cy * P2_LEVEL_WIDTH) + cx]);
}

PushableObject::PushableObject(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:GameObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile),
	AnimableObject(x, y), m_defused(false)
{}

AnimableObject::AnimableObject(uint8_t ax, uint8_t ay)
	:m_anim_x(ax * P2_TILE_WIDTH), m_anim_y(ay * P2_TILE_HEIGHT),
	m_anim_index(0), m_anim_state(0)
{}

Ball::Ball(const TileSet *sprites, const uint8_t *tilemap,
	uint8_t x, uint8_t y, GameObject **objects,
	uint8_t first_floor_tile, uint8_t first_cross_tile)
	:PushableObject(sprites, tilemap, x, y, objects, first_floor_tile, first_cross_tile),
	m_rolling(false), m_speed(PUSH_SPEED), m_roll_momentum(0)
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
	AnimableObject(x, y), m_speed(PLAYER_SPEED), m_busy(false), m_push_momentum(0), m_straining(false)
{}

bool AnimableObject::arrived(uint8_t x, uint8_t y) const
{
	return ((m_anim_x == (uint32_t)x * P2_TILE_WIDTH) && (m_anim_y == (uint32_t)y * P2_TILE_HEIGHT));
}

void AnimableObject::slideTo(uint8_t x, uint8_t y, uint32_t speed)
{
	if (m_anim_x > (uint32_t)x * P2_TILE_WIDTH)
		m_anim_x -= speed;
	else if (m_anim_x < (uint32_t)x * P2_TILE_WIDTH)
		m_anim_x += speed;
	else if (m_anim_y > (uint32_t)y * P2_TILE_HEIGHT)
		m_anim_y -= speed;
	else if (m_anim_y < (uint32_t)y * P2_TILE_HEIGHT)
		m_anim_y += speed;
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
	int cx = m_x;
	int cy = m_y;
	switch (d)
	{
		case Up:
			for (cy = m_y - 1; cy > 0; --cy)
			{
				if (!tileIsEmpty(m_x, cy) || m_objects[(cy * P2_LEVEL_WIDTH) + m_x])
				{
					++cy;
					break;
				}
			}
			break;
		case Down:
			for (cy = m_y + 1; cy < P2_LEVEL_HEIGHT; ++cy)
			{
				if (!tileIsEmpty(m_x, cy) || m_objects[(cy * P2_LEVEL_WIDTH) + m_x])
				{
					--cy;
					break;
				}
			}
			break;
		case Left:
			for (cx = m_x - 1; cx > 0; --cx)
			{
				if (!tileIsEmpty(cx, m_y) || m_objects[(m_y * P2_LEVEL_WIDTH) + cx])
				{
					++cx;
					break;
				}
			}
			break;
		case Right:
			for (cx = m_x + 1; cx < P2_LEVEL_WIDTH; ++cx)
			{
				if (!tileIsEmpty(cx, m_y) || m_objects[(m_y * P2_LEVEL_WIDTH) + cx])
				{
					--cx;
					break;
				}
			}
	}
	m_objects[(m_y * P2_LEVEL_WIDTH) + m_x] = NULL;
	m_objects[(cy * P2_LEVEL_WIDTH) + cx] = this;
	m_x = cx;
	m_y = cy;
	m_rolling = true;
	m_speed = PUSH_SPEED;
	m_roll_momentum = 0;
}

bool Box::rolls() const
{
	return false;
}

void Box::push(Direction d)
{
	// Move one square in given direction
	int cx = m_x;
	int cy = m_y;
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
	m_objects[(m_y * P2_LEVEL_WIDTH) + m_x] = NULL;
	m_objects[(cy * P2_LEVEL_WIDTH) + cx] = this;
	m_x = cx;
	m_y = cy;
}

void Ball::render(SDL_Surface *screen)
{
	if (m_rolling)
	{
		slideTo(m_x, m_y, m_speed);
		if (arrived(m_x, m_y))
			m_rolling = false;
		// First half of first square is slid to at
		// half normal speed, the momentum picks up
		if ((m_roll_momentum += m_speed) >= P2_TILE_WIDTH / 2)
			m_speed = ROLL_SPEED;
	}
	// Check (when arrived) whether destination is a cross, and defuse
	if (!m_rolling && !m_defused && tileIsCross(m_x, m_y))
	{
		--objects_left;
		m_defused = true;
	}
	if (m_defused && !tileIsCross(m_x, m_y))
	{
		// We've been pushed off a cross, arrived or otherwise
		++objects_left;
		m_defused = false;
	}
	// 8-14 = on fire, 15-19 = defused
	// minus 8, that makes 0-6 = on fire, 7-11 defused
	// double up frame indexes to slow down animation rate
	switch (m_anim_state)
	{
		case 0:
			// on fire - animation looping upwards
			if (++m_anim_index == 13)
			{
				if (m_defused)
					m_anim_state = 2;
				else
				{
					m_anim_state = 1;
					// Don't stay at loop end frame
					// for three frames in a row
					--m_anim_index;
				}
			}
			break;
		case 1:
			// on fire - animation looping downwards
			if (--m_anim_index == 0)
			{
				m_anim_state = 0;
				// Don't stay at loop end frame
				// for three frames in a row
				++m_anim_index;
			}
			break;
		case 2:
			// defused - looping upwards
			if (++m_anim_index == 23)
			{
				m_anim_state = 3;
				// Don't stay at loop end frame
				// for three frames in a row
				--m_anim_index;
			}
			break;
		case 3:
			// defused - looping downwards
			if (--m_anim_index == 14)
			{
				if (m_defused)
				{
					m_anim_state = 2;
					// Don't stay at loop end frame
					// for three frames in a row
					++m_anim_index;
				}
				else
					m_anim_state = 1;
			}
	}
	SDL_Rect rect = {
		m_anim_x, m_anim_y,
		0, 0
	};
	// frame indexes are doubled to slow down animation rates
	// halve them before accessing the sprite array
	SDL_Surface *sprite = (*m_sprites)[(m_anim_index / 2) + 8];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
}

void Box::render(SDL_Surface *screen)
{
	slideTo(m_x, m_y, PUSH_SPEED);
	if (!m_defused && tileIsCross(m_x, m_y) && arrived(m_x, m_y))
	{
		// We've arived on a cross
		--objects_left;
		m_defused = true;
	}

	if (m_defused && !tileIsCross(m_x, m_y))
	{
		// We've been pushed off a cross, arrived or otherwise
		++objects_left;
		m_defused = false;
	}

	SDL_Rect rect = {
		m_anim_x, m_anim_y,
		0, 0
	};
	// animation frame indexes:
	// 0-6 = on fire, 7 = defused
	switch (m_anim_state)
	{
		case 0:
			// on fire - animation looping forwards
			// double up frame indexes to slow down animation rate
			if (++m_anim_index == 13)
			{
				if (m_defused)
					m_anim_state = 2;
				else
				{
					m_anim_state = 1;
					// Don't stay at loop end frame
					// for three frames in a row
					--m_anim_index;
				}
			}
			break;
		case 1:
			// on fire - animation looping backwards
			if (--m_anim_index == 0)
			{
				m_anim_state = 0;
				// Don't stay at loop end frame
				// for three frames in a row
				++m_anim_index;
			}
			break;
		case 2:
			// defused
			m_anim_index = 14;
			if (!m_defused)
			{
				--m_anim_index;
				m_anim_state = 1;
			}
	}
	// frame indexes are doubled to slow down animation rates
	// halve them before accessing the sprite array
	SDL_Surface *sprite = (*m_sprites)[m_anim_index / 2];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
}

void Player::render(SDL_Surface *screen)
{
	if (m_busy)
	{
		slideTo(m_x, m_y, m_speed);

		if (arrived(m_x, m_y))
		{
			// We've arrived - we have finished pushing for now
			m_speed = PLAYER_SPEED;
			m_busy = false;
		}

		// If we're pushing a rolling object, eventually
		// it picks up speed and we can move more freely
		if (m_push_momentum > 0)
		{
			if ((m_push_momentum -= PUSH_SPEED) == 0)
			{
				m_speed = PLAYER_SPEED;
				m_straining = false;
			}
		}
	}
	// Animate up/down/left/right loops
	// All are 6 frames long, but go too quickly at 30fps,
	// so count to 12 and only step every other frame
	if (++m_anim_index == 12)
		m_anim_index = 0;
	SDL_Rect rect = {
		m_anim_x, m_anim_y,
		0, 0
	};
	SDL_Surface *sprite = (*m_sprites)[(m_anim_index / 2) + m_anim_state + (m_straining ? 24 : 0)];
	SDL_BlitSurface(sprite, NULL, screen, &rect);
	// If player pushed against a wall, only stay in the left/right/up/down
	// animation for one frame, unless they keep the key held down
	if (!m_busy)
	{
		m_anim_state = 0;
		m_straining = false;
	}
}

void Player::move(Direction d)
{
	// Can't change direction whilst moving
	if (m_busy)
		return;

	int cx = m_x;
	int cy = m_y;
	switch (d)
	{
		case Up:
			if (cy > 0)
				--cy;
			// First frame of up animation
			m_anim_state = 6;
			break;
		case Down:
			if (cy < P2_LEVEL_HEIGHT)
				++cy;
			// First frame of down animation
			m_anim_state = 12;
			break;
		case Left:
			if (cx > 0)
				--cx;
			// First frame of left animation
			m_anim_state = 18;
			break;
		case Right:
			if (cx < P2_LEVEL_WIDTH)
				++cx;
			// First frame of right animation
			m_anim_state = 24;
	}
	// Are we away from the edges?
	if (cx != m_x || cy != m_y)
	{
		// Is the space blocked by a wall?
		if (tileIsEmpty(cx, cy))
		{
			// Is there a pushable object there?
			GameObject *o = m_objects[(cy * P2_LEVEL_WIDTH) + cx];
			if (o)
			{
				// Use strain animations up against objects,
				// movable or otherwise
				m_straining = true;
				if (((PushableObject*)o)->canMove(d))
				{
					((PushableObject*)o)->push(d);
					// We're straining - reduce movement speed
					m_speed = PUSH_SPEED;
					// But maybe only for half a square
					if (((PushableObject*)o)->rolls())
						m_push_momentum = P2_TILE_WIDTH / 2;
				}
				else
					return;
			}
			// We can move
			m_objects[(m_y * P2_LEVEL_WIDTH) + m_x] = NULL;
			m_objects[(cy * P2_LEVEL_WIDTH) + cx] = this;
			m_x = cx;
			m_y = cy;
			m_busy = true;
		}
		else
		{
			// Use strain animations up against walls
			m_straining = true;
		}
	}
}
