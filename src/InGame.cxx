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
#include <sstream>
#include <cmath>

// System

// Library

// Local
#include "InGame.hxx"
#include "PauseMenu.hxx"
#include "Score.hxx"
#include "MainMenu.hxx"


//
// Implementation
//

#define BONUS_COUNTER_RATE 9.05f

InGame::InGame(const Alphabet &a, const LevelSet &l, int level, uint32_t score)
	: GameLoop(a, l), m_level(level), m_score(score), m_advance(false),
	  m_player(NULL), m_name_surf(NULL), m_background_surf(NULL),
	  m_score_surf(NULL), m_int_bonus_counter(-1), m_bonus_surf(NULL)
{
	// Render level name into a surface
	m_name_surf = a.renderWord(l[level].name,
		l[level].name_colour[0],
		l[level].name_colour[1],
		l[level].name_colour[2]);

	// Render current score into a surface
	std::ostringstream score_str;
	score_str << m_score;
	m_score_surf = a.renderWord(score_str.str(), 215, 215, 215);

	// Set initial value of bonus counter
	m_bonus_counter = l[level].bonus;

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
					l.firstFloorTile(), l.firstCrossTile(), m_objects_left);
				m_player = (Player*) *o;
				break;
			case 1:
				*o = new Box(&(l.getSprites()),
					l[level].tilemap, s->x, s->y, m_object_array,
					l.firstFloorTile(), l.firstCrossTile(), m_objects_left);
				break;
			case 2:
				*o = new Ball(&(l.getSprites()),
					l[level].tilemap, s->x, s->y, m_object_array,
					l.firstFloorTile(), l.firstCrossTile(), m_objects_left);
		}
		m_objects.emplace_back(*o);
	}

	// Set the number of objects in the current level,
	// for keeping track of when the level is completed.
	// One object is the player.
	m_objects_left = l[level].num_sprites - 1;

	// Create pre-rendered surface containing game background
	const uint8_t *tilemap = m_levelset[m_level].tilemap;
	m_background_surf = SDL_DisplayFormat(SDL_GetVideoSurface());
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
				NULL, m_background_surf, &rect);
		}
	}
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

	// Pause when escape is pressed or app loses focus
	if (kbdstate[SDLK_ESCAPE] || !(SDL_GetAppState() & SDL_APPINPUTFOCUS))
		return false;

	// Render background & game objects
	SDL_BlitSurface(m_background_surf, NULL, screen, NULL);
	for (auto i = m_objects.cbegin(); i != m_objects.cend(); ++i)
	{
		(*i)->render(screen, elapsed);
	}

	// Render level name & score
	SDL_Rect rect = {
		50, 320, 0, 0
	};
	SDL_BlitSurface(m_name_surf, NULL, screen, &rect);
	rect.x = (screen->w - 50) - m_score_surf->w;
	rect.y = 320; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(m_score_surf, NULL, screen, &rect);

	// Render current bonus counter value
	// RGB values based on colours from a screenshot
	if (m_int_bonus_counter)
	{
		m_bonus_counter -= BONUS_COUNTER_RATE * elapsed;
		if (m_int_bonus_counter != (int) floorf(m_bonus_counter))
		{
			m_int_bonus_counter = (int) floorf(m_bonus_counter);

			std::ostringstream bonus_str;
			bonus_str << m_int_bonus_counter;

			if (m_bonus_surf)
				SDL_FreeSurface(m_bonus_surf);

			m_bonus_surf = m_alphabet.renderWord(bonus_str.str(),
				62, 253, 231);
		}
	}

	rect.x = 448; rect.y = 4; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(m_bonus_surf, NULL, screen, &rect);

	if (m_objects_left)
		return true;
	else
	{
		m_score += m_int_bonus_counter;
		if (m_score > Score::high)
			Score::high = m_score;
		m_advance = true;
		return false;
	}
}

std::unique_ptr<GameLoopFactory> InGame::nextLoop()
{
	// If we get here, update() must have returned false.
	// It will do this either because the player should
	// advance to the next level, or because the game has
	// been paused.
	if (m_advance)
	{
		GameLoopFactory *f = 0;
		if ((size_t)(m_level + 1) == m_levelset.size())
		{
			// For now, if the player has finished the last
			// level, just go back to the main menu.
			f = new MainMenuFactory();
		}
		else
		{
			f = new InGameFactory();
			((InGameFactory*)f)->level = m_level + 1;
			((InGameFactory*)f)->score = m_score;
		}
		f->a = &m_alphabet;
		f->l = &m_levelset;
		return std::unique_ptr<GameLoopFactory>(f);
	}
	else
	{
		PauseFactory *f(new PauseFactory());
		f->a = &m_alphabet;
		f->l = &m_levelset;
		f->paused_loop = shared_from_this();
		return std::unique_ptr<GameLoopFactory>(f);
	}
}

InGame::~InGame()
{
	SDL_FreeSurface(m_name_surf);
	SDL_FreeSurface(m_background_surf);
	SDL_FreeSurface(m_score_surf);
	SDL_FreeSurface(m_bonus_surf);
}

std::shared_ptr<GameLoop> InGameFactory::operator() ()
{
	return std::shared_ptr<GameLoop>(new InGame(*a, *l, level, score));
}
