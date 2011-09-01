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

#ifndef HXX_MAINMENU
#define HXX_MAINMENU

// GameLoop derivative for displaying the main menu
class MainMenu: public GameLoop
{
	public:
		MainMenu(const Alphabet &a, const LevelSet &l);
		~MainMenu();

		bool update(float elapsed, const Uint8 *kbdstate,
			SDL_Surface *screen);

		std::unique_ptr<GameLoopFactory> nextLoop();

	private:
		int m_selected_item;
		SDL_Surface *m_menu_items[4];

		SDL_Surface *m_background_surf;

		int m_kbdstate_size;
		Uint8 *m_old_kbdstate;

		GameLoopFactory *m_next_loop;
};

#endif
