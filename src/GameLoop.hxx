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

#ifndef HXX_GAMELOOP
#define HXX_GAMELOOP

class GameLoop;

// Base struct for GameLoop-derived class factories
struct GameLoopFactory
{
	const Alphabet *a;
	const LevelSet *l;

	virtual std::shared_ptr<GameLoop> operator() () = 0;
};

// Base class for switchable main loop update function
class GameLoop: public std::enable_shared_from_this<GameLoop>
{
	public:
		GameLoop(const Alphabet &a, const LevelSet &l)
			: m_alphabet(a), m_levelset(l), m_tileset(l.getTiles())
		{};

		virtual ~GameLoop() {};

		// Update game state and render current frame.
		// Return false to indicate that a new loop
		// should be swapped in.  If the state of the
		// current loop is to be preserved, have the new loop
		// take a shared_ptr to the current one as an argument.
		virtual bool update(float elapsed, const Uint8 *kbdstate,
			SDL_Surface *screen) = 0;

		// Return a shared_ptr to the next game loop factory.
		// This will be called after update() has returned false.
		// If it returns a pointer to 0, quit the game.
		// The current game loop will be destroyed *before*
		// calling operator() on the factory for the next loop.
		virtual std::unique_ptr<GameLoopFactory> nextLoop() = 0;

	protected:
		const Alphabet &m_alphabet;
		const LevelSet &m_levelset;
		const TileSet &m_tileset;
};

#endif
