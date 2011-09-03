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
#include <cstring>

// System

// Library

// Local
#include "Credits.hxx"
#include "MainMenu.hxx"


//
// Implementation
//

Credits::Credits(const Alphabet &a, const LevelSet &l)
	: GameLoop(a, l), m_background_surf(NULL), m_old_kbdstate(NULL)
{
	// Render main menu background
	const uint8_t *tilemap = m_levelset.getTitleScreen();
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

	// Render credits text
	SDL_Surface *title = a.renderWord("Pushy II", 192, 192, 192);
	SDL_Surface *from_fish = a.renderWord("from FISH", 0, 255, 255);
	SDL_Surface *net = a.renderWord("net", 0, 255, 255);
	SDL_Surface *graphics = a.renderWord("Graphics", 192, 128, 0);
	SDL_Surface *and_levels = a.renderWord("and Levels by:", 128, 0, 192);
	SDL_Surface *rfredw = a.renderWord("R-Fred-W", 192, 192, 0);
	SDL_Surface *code_by = a.renderWord("Code by:", 192, 128, 0);
	SDL_Surface *phil = a.renderWord("Philip Allison", 192, 192, 0);
		
	SDL_Rect rect;
	rect.x = (Sint16)(320 - (title->w / 2));
	rect.y = 0; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(title, NULL, m_background_surf, &rect);

	int fw = from_fish->w + 12 + net->w;
	rect.x = (Sint16)(320 - (fw / 2));
	rect.y = 60; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(from_fish, NULL, m_background_surf, &rect);

	rect.x = (Sint16)((320 - (fw / 2)) + from_fish->w + 12);
	rect.y = 48; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(net, NULL, m_background_surf, &rect);

	rect.x = 40; rect.y = 120; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(graphics, NULL, m_background_surf, &rect);

	rect.x = 80; rect.y = 160; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(and_levels, NULL, m_background_surf, &rect);

	rect.x = (Sint16)(320 - (rfredw->w / 2));
	rect.y = 210; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(rfredw, NULL, m_background_surf, &rect);

	rect.x = 40; rect.y = 260; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(code_by, NULL, m_background_surf, &rect);

	rect.x = (Sint16)(320 - (phil->w / 2));
	rect.y = 310; rect.w = 0; rect.h = 0;
	SDL_BlitSurface(phil, NULL, m_background_surf, &rect);

	SDL_FreeSurface(title);
	SDL_FreeSurface(from_fish);
	SDL_FreeSurface(net);
	SDL_FreeSurface(graphics);
	SDL_FreeSurface(and_levels);
	SDL_FreeSurface(rfredw);
	SDL_FreeSurface(code_by);
	SDL_FreeSurface(phil);

	// Store current keyboard state, and size of keyboard state array.
	// This is so that later we can process keypresses separate from
	// keys which were already held down when entering the menu.
	const Uint8 *kbdstate = SDL_GetKeyState(&m_kbdstate_size);
	m_old_kbdstate = new Uint8[m_kbdstate_size];
	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);
}

Credits::~Credits()
{
	SDL_FreeSurface(m_background_surf);
	delete[] m_old_kbdstate;
}

bool Credits::update(float elapsed, const Uint8 *kbdstate, SDL_Surface *screen)
{
	SDL_BlitSurface(m_background_surf, NULL, screen, NULL);

	if ((kbdstate[SDLK_ESCAPE]
		 || kbdstate[SDLK_SPACE]
		 || kbdstate[SDLK_RETURN]
		 || kbdstate[SDLK_KP_ENTER])
		&& !(m_old_kbdstate[SDLK_ESCAPE]
		 || m_old_kbdstate[SDLK_SPACE]
		 || m_old_kbdstate[SDLK_RETURN]
		 || m_old_kbdstate[SDLK_KP_ENTER]))
	{
		return false;
	}

	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);
	return true;
}

std::unique_ptr<GameLoopFactory> Credits::nextLoop()
{
	GameLoopFactory *f = new MainMenuFactory();
	f->a = &m_alphabet;
	f->l = &m_levelset;
	return std::unique_ptr<GameLoopFactory>(f);
}

std::shared_ptr<GameLoop> CreditsFactory::operator() ()
{
	return std::shared_ptr<GameLoop>(new Credits(*a, *l));
}
