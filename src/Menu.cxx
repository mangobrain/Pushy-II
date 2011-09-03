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
#include <cstdarg>

// System

// Library

// Local
#include "Menu.hxx"


//
// Implementation
//

Menu::Menu(const Alphabet &a, const LevelSet &l)
	: GameLoop(a, l), m_selected_item(0),
	  m_y_offset(0), m_background_surf(NULL),
	  m_old_kbdstate(NULL), m_next_loop(0)
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

	// Render title at the top
	SDL_Surface *title = a.renderWord("Pushy II", 192, 192, 192);
	SDL_Rect rect = {
		(Sint16)(320 - (title->w / 2)),
		40, 0, 0
	};
	SDL_BlitSurface(title, NULL, m_background_surf, &rect);
	SDL_FreeSurface(title);

	// Store current keyboard state, and size of keyboard state array.
	// This is so that later we can process keypresses separate from
	// keys which were already held down when entering the menu.
	const Uint8 *kbdstate = SDL_GetKeyState(&m_kbdstate_size);
	m_old_kbdstate = new Uint8[m_kbdstate_size];
	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);
}

Menu::~Menu()
{
	for (auto i = m_menu_items.begin(); i != m_menu_items.end(); ++i)
		SDL_FreeSurface(*i);
	SDL_FreeSurface(m_background_surf);
	delete[] m_old_kbdstate;
}

void Menu::setMenuItems(int count, ...)
{
	va_list ap;

	// Render text of menu items onto surfaces
	m_menu_items.reserve(count);
	int h = 0;
	va_start(ap, count);
	for (int i = 0; i < count; ++i)
	{
		const char * word = va_arg(ap, const char*);
		int r = va_arg(ap, int);
		int g = va_arg(ap, int);
		int b = va_arg(ap, int);

		m_menu_items.push_back(m_alphabet.renderWord(word, r, g, b));

		// Calculate total height of menu - used later for vertical centreing
		h += m_menu_items.back()->h;
	}
	va_end(ap);

	// Calculate Y offset for menu rendering
	// Window height is 384 pixels, half of which is 192,
	// plus a small offset to account for the title at the top
	m_y_offset = 205 - (h / 2);
}

bool Menu::update(float elapsed, const Uint8 *kbdstate, SDL_Surface *screen)
{
	SDL_BlitSurface(m_background_surf, NULL, screen, NULL);

	// Main menu visible.  Render menu items.
	Sint16 yoff = m_y_offset;
	for (size_t i = 0; i < m_menu_items.size(); ++i)
	{
		if (i == (size_t)m_selected_item)
			SDL_SetAlpha(m_menu_items[i], SDL_SRCALPHA, 255);
		else
			SDL_SetAlpha(m_menu_items[i], SDL_SRCALPHA, 127);

		SDL_Rect rect = {
			(Sint16)(320 - (m_menu_items[i]->w / 2)),
			yoff, 0, 0
		};

		SDL_BlitSurface(m_menu_items[i], NULL, screen, &rect);
		yoff += m_menu_items[i]->h;
	}

	// Handle menu navigation
	if (kbdstate[SDLK_UP] && !m_old_kbdstate[SDLK_UP])
	{
		if (--m_selected_item < 0)
			m_selected_item = m_menu_items.size() - 1;
	}
	else if (kbdstate[SDLK_DOWN] && !m_old_kbdstate[SDLK_DOWN])
	{
		if (++m_selected_item == (int)m_menu_items.size())
			m_selected_item = 0;
	}
	else if (
		(kbdstate[SDLK_RETURN] || kbdstate[SDLK_KP_ENTER] || kbdstate[SDLK_SPACE])
		&& !(m_old_kbdstate[SDLK_RETURN] || m_old_kbdstate[SDLK_KP_ENTER] || m_old_kbdstate[SDLK_SPACE]))
	{
		m_next_loop = loopForItem(m_selected_item);
		if (m_next_loop)
		{
			m_next_loop->a = &m_alphabet;
			m_next_loop->l = &m_levelset;
		}
		return false;
	}

	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);

	return true;
}

std::unique_ptr<GameLoopFactory> Menu::nextLoop()
{
	return std::unique_ptr<GameLoopFactory>(m_next_loop);
}
