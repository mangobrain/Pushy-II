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
#include "PasswordEntry.hxx"
#include "InGame.hxx"
#include "MainMenu.hxx"


//
// Implementation
//

PasswordEntry::PasswordEntry(const Alphabet &a, const LevelSet &l)
	: GameLoop(a, l), m_background_surf(NULL),
	  m_old_kbdstate(NULL), m_password_surf(NULL), m_next_loop(0)
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

	SDL_Surface *p = a.renderWord("Password:", 255, 0, 255);
	SDL_Rect rect = {
		(Sint16)(320 - (p->w / 2)), 100, 0, 0
	};
	SDL_BlitSurface(p, NULL, m_background_surf, &rect);
	SDL_FreeSurface(p);

	// Store current keyboard state, and size of keyboard state array.
	// This is so that later we can process keypresses separate from
	// keys which were already held down when entering the menu.
	const Uint8 *kbdstate = SDL_GetKeyState(&m_kbdstate_size);
	m_old_kbdstate = new Uint8[m_kbdstate_size];
	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);
}

PasswordEntry::~PasswordEntry()
{
	SDL_FreeSurface(m_background_surf);
	if (m_password_surf)
		SDL_FreeSurface(m_password_surf);
	delete[] m_old_kbdstate;
}

bool PasswordEntry::update(float elapsed, const Uint8 *kbdstate, SDL_Surface *screen)
{
	SDL_BlitSurface(m_background_surf, NULL, screen, NULL);

	bool password_changed = false;

	// Loop over letter keys and see if any have been pressed.
	// Modify the current password string accordingly.
	// Impose a width limit to prevent overrunning the edges
	// of the screen - allow the longest password in the base
	// Pushy II level set, though!
	if (m_password.length() < 9)
	{
		for (int i = SDLK_a; i <= SDLK_z; ++i)
		{
			if (kbdstate[i] && !m_old_kbdstate[i])
			{
				char c = (i - SDLK_a) + 97;
				if (m_password.empty())
					c -= 32;
				m_password.append(1, c);
				password_changed = true;
				break;
			}
		}
	}

	// Remove the last character on delete/backspace
	if (!m_password.empty()
		&& (kbdstate[SDLK_BACKSPACE] || kbdstate[SDLK_DELETE])
		&& !(m_old_kbdstate[SDLK_BACKSPACE] || m_old_kbdstate[SDLK_DELETE]))
	{
		m_password.resize(m_password.length() - 1);
		password_changed = true;
	}

	if (password_changed)
	{
		if (m_password_surf)
		{
			SDL_FreeSurface(m_password_surf);
			m_password_surf = NULL;
		}

		if (!m_password.empty())
		{
			m_password_surf =
				m_alphabet.renderWord(m_password, 255, 255, 0);
		}
	}

	// Render the current password string
	if (m_password_surf)
	{
		SDL_Rect rect = {
			(Sint16)(320 - (m_password_surf->w / 2)), 200, 0, 0
		};
		SDL_BlitSurface(m_password_surf, NULL, screen, &rect);
	}

	// If enter is pressed, see if there is a level with the
	// current password - if not, go back to the main menu
	if ((kbdstate[SDLK_RETURN] || kbdstate[SDLK_KP_ENTER])
		&& !(m_old_kbdstate[SDLK_RETURN] || m_old_kbdstate[SDLK_KP_ENTER]))
	{
		for (size_t i = 0; i < m_levelset.size(); ++i)
		{
			if (m_levelset[i].name == m_password)
			{
				m_next_loop = new InGameFactory();
				((InGameFactory*)m_next_loop)->score = 0;
				((InGameFactory*)m_next_loop)->level = i;
				break;
			}
		}
		if (!m_next_loop)
		{
			m_next_loop = new MainMenuFactory();
		}
		m_next_loop->a = &m_alphabet;
		m_next_loop->l = &m_levelset;
		return false;
	}

	memcpy(m_old_kbdstate, kbdstate, m_kbdstate_size);
	return true;
}

std::unique_ptr<GameLoopFactory> PasswordEntry::nextLoop()
{
	return std::unique_ptr<GameLoopFactory>(m_next_loop);
}

std::shared_ptr<GameLoop> PasswordEntryFactory::operator() ()
{
	return std::shared_ptr<GameLoop>(new PasswordEntry(*a, *l));
}
