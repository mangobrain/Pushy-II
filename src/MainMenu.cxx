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
#include <cstring>

// System

// Library
#include <SDL.h>

// Local
#include "Constants.hxx"
#include "Alphabet.hxx"
#include "TileSet.hxx"
#include "LevelSet.hxx"
#include "GameLoop.hxx"
#include "MainMenu.hxx"
#include "GameObjects.hxx"
#include "InGame.hxx"


//
// Implementation
//

MainMenu::MainMenu(const Alphabet &a, const LevelSet &l)
	: Menu(a, l)
{
	// Set main menu items
	setMenuItems(4,
		"Start Game", 255, 255, 0,
		"Password", 0, 255, 0,
		"Credits", 255, 127, 0,
		"Quit Game", 255, 0, 0);
}

GameLoopFactory * MainMenu::loopForItem(int item)
{
	GameLoopFactory *r = 0;

	switch (item)
	{
		case 0:
			// New game
			r = new InGameFactory();
			r->a = &m_alphabet;
			r->l = &m_levelset;
			((InGameFactory*)r)->score = 0;
			((InGameFactory*)r)->level = 0;
			break;
		case 1:
			// TODO Password
			break;
		case 2:
			// TODO Credits
			break;
		default:
			// End game
			break;
	}

	return r;
}

std::shared_ptr<GameLoop> MainMenuFactory::operator() ()
{
	return std::shared_ptr<GameLoop>(new MainMenu(*a, *l));
}
