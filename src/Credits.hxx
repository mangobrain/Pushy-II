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

#ifndef HXX_CREDITS
#define HXX_CREDITS

#include "GameLoop.hxx"

// GameLoop derivative for displaying credits
class Credits: public GameLoop
{
	public:
		Credits(const Alphabet &a, const LevelSet &l);
		~Credits();

		bool update(float elapsed, const Uint8 *kbdstate,
			SDL_Surface *screen);

		std::unique_ptr<GameLoopFactory> nextLoop();

	private:
		SDL_Surface *m_background_surf;

		int m_kbdstate_size;
		Uint8 *m_old_kbdstate;
};

struct CreditsFactory: public GameLoopFactory
{
	std::shared_ptr<GameLoop> operator() ();
};

#endif
