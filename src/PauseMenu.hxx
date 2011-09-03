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

#ifndef HXX_PAUSEMENU
#define HXX_PAUSEMENU

#include "Menu.hxx"

// GameLoop derivative for displaying the pause menu
class PauseMenu: public Menu
{
	public:
		PauseMenu(const Alphabet &a, const LevelSet &l,
			std::shared_ptr<GameLoop> paused_loop);
		
	private:
		GameLoopFactory * loopForItem(int item);

		std::shared_ptr<GameLoop> m_paused_loop;
		bool m_unpause;
};

// Factory class for PauseMenu
// Set paused_loop to point to the InGame being paused
struct PauseFactory: public GameLoopFactory
{
	std::shared_ptr<GameLoop> paused_loop;

	std::shared_ptr<GameLoop> operator() ();
};

#endif
