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
#include "PauseMenu.hxx"
#include "InGame.hxx"
#include "MainMenu.hxx"


//
// Implementation
//

// Dummy "factory" class for returning the paused InGame when un-pausing
struct UnpauseFactory: public GameLoopFactory
{
	std::shared_ptr<GameLoop> paused_loop;

	std::shared_ptr<GameLoop> operator() ()
	{
		return paused_loop;
	};
};

PauseMenu::PauseMenu(const Alphabet &a, const LevelSet &l,
	std::shared_ptr<GameLoop> paused_loop)
	: Menu(a, l), m_paused_loop(paused_loop), m_unpause(false)
{
	// Set pause menu items
	setMenuItems(3,
		"Continue", 0, 0, 255,
		"Retry", 255, 0, 255,
		"Quit", 0, 255, 0);
}

GameLoopFactory * PauseMenu::loopForItem(int item)
{
	GameLoopFactory *r = 0;

	switch (item)
	{
		case 0:
			// Continue
			r = new UnpauseFactory();
			r->a = &m_alphabet;
			r->l = &m_levelset;
			((UnpauseFactory*)r)->paused_loop = m_paused_loop;
			break;
		case 1:
			{
				// Retry - destroy old InGame object,
				// then create a new one for the same level
				InGame *i = (InGame*)(m_paused_loop.get());
				int level = i->getLevel();
				uint32_t score = i->getScore();
				m_paused_loop.reset();
				r = new InGameFactory();
				r->a = &m_alphabet;
				r->l = &m_levelset;
				((InGameFactory*)r)->score = score;
				((InGameFactory*)r)->level = level;
			}
			break;

		default:
			// Quit to main menu
			r = new MainMenuFactory();
			r->a = &m_alphabet;
			r->l = &m_levelset;
			break;
	}

	return r;
}

std::shared_ptr<GameLoop> PauseFactory::operator() ()
{
	return std::shared_ptr<GameLoop>(new PauseMenu(*a, *l, paused_loop));
}
