// Copyright 2011 Philip Allison

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
#include <memory>

// System

// Library
#include <SDL.h>

// Local
#include "Constants.hxx"
#include "TileSet.hxx"
#include "LevelSet.hxx"
#include "Alphabet.hxx"
#include "GameLoop.hxx"
#include "GameObjects.hxx"
#include "InGame.hxx"

//
// Implementation
//

extern int objects_left;

InGame::InGame(const Alphabet &a, const LevelSet &l, int level)
	: GameLoop(a, l), m_level(level), m_score(0), m_advance(false),
	  m_player(NULL), m_name_surf(NULL)
{
	// Render level name into a surface
	m_name_surf = a.renderWord(l[level].name,
		l[level].name_colour[0],
		l[level].name_colour[1],
		l[level].name_colour[2]);

	// Create the game objects for the current level,
	// placing them in the array representing the squares.
	m_objects.reserve(l[level].num_sprites);
	memset(m_object_array, 0, sizeof(m_object_array));
	for (uint8_t i = 0; i < l[level].num_sprites; ++i)
	{
		const SpriteInfo *s = &(l[level].spriteinfo[i]);
		GameObject **o = &(m_object_array[(s->y * P2_LEVEL_WIDTH) + s->x]);
		switch (s->index)
		{
			case 0:
				*o = new Player(&(l.getPlayerSprites()),
					l[level].tilemap, s->x, s->y, m_object_array,
					l.firstFloorTile(), l.firstCrossTile());
				m_player = (Player*) *o;
				break;
			case 1:
				*o = new Box(&(l.getSprites()),
					l[level].tilemap, s->x, s->y, m_object_array,
					l.firstFloorTile(), l.firstCrossTile());
				break;
			case 2:
				*o = new Ball(&(l.getSprites()),
					l[level].tilemap, s->x, s->y, m_object_array,
					l.firstFloorTile(), l.firstCrossTile());
		}
		m_objects.emplace_back(*o);
	}

	// Set the number of objects in the current level,
	// for keeping track of when the level is completed.
	// One object is the player.
	objects_left = l[level].num_sprites - 1;
}

bool InGame::update(float elapsed, const Uint8 *kbdstate, SDL_Surface *screen)
{
	// Handle keypresses separately
	// (we don't care about explicit presses/releases,
	// just which keys are being held down)
	if (kbdstate[SDLK_UP])
		m_player->move(Up);
	else if (kbdstate[SDLK_DOWN])
		m_player->move(Down);
	else if (kbdstate[SDLK_LEFT])
		m_player->move(Left);
	else if (kbdstate[SDLK_RIGHT])
		m_player->move(Right);
	if (kbdstate[SDLK_ESCAPE])
		return false;

	// Render background & game objects
	const uint8_t *tilemap = m_levelset[m_level].tilemap;
	for (int y = 0; y < P2_LEVEL_HEIGHT; ++y)
	{
		for (int x = 0; x < P2_LEVEL_WIDTH; ++x)
		{
			SDL_Rect rect = {
				(Sint16)(x * P2_TILE_WIDTH),
				(Sint16)(y * P2_TILE_HEIGHT),
				0, 0
			};
			SDL_BlitSurface(m_tileset[tilemap[(y * P2_LEVEL_WIDTH) + x]],
				NULL, screen, &rect);
		}
	}
	for (auto i = m_objects.cbegin(); i != m_objects.cend(); ++i)
	{
		(*i)->render(screen, elapsed);
	}

	// Render level name
	SDL_Rect rect = {
		50, 320, 0, 0
	};
	SDL_BlitSurface(m_name_surf, NULL, screen, &rect);

	if (objects_left)
		return true;
	else
	{
		m_advance = true;
		return false;
	}
}

std::shared_ptr<GameLoop> InGame::nextLoop()
{
	if (m_advance)
	{
		return std::shared_ptr<GameLoop>(
			new InGame(m_alphabet, m_levelset, m_level + 1)
		);
	}
	else
		return std::shared_ptr<GameLoop>();
}

InGame::~InGame()
{
	SDL_FreeSurface(m_name_surf);
}