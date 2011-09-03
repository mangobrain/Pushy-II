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

#ifndef HXX_INGAME
#define HXX_INGAME

#include "GameLoop.hxx"
#include "GameObjects.hxx"

// GameLoop-derived class for main in-level gameplay
class InGame: public GameLoop
{
	public:
		InGame(const Alphabet &a, const LevelSet &l, int level = 0,
			uint32_t score = 0);
		~InGame();

		bool update(float elapsed, const Uint8 *kbdstate, SDL_Surface *screen);
		std::unique_ptr<GameLoopFactory> nextLoop();

		int getLevel() const
		{
			return m_level;
		};

		uint32_t getScore() const
		{
			return m_score;
		};

	private:
		int m_level;
		uint32_t m_score;
		int m_objects_left;
		bool m_advance;

		// Array of pointers to game objects, one per square.
		// Each game object has a pointer to it somewhere in this
		// array, their positions managed by the GameObjects themselves
		// as they move around (they contain a pointer to this array).
		GameObject* m_object_array[P2_LEVEL_WIDTH * P2_LEVEL_HEIGHT];

		// Each game object is also stored here, so that they can be
		// iterated over without having to walk the whole array above,
		// and so that they get deleted on ~InGame().
		std::vector<std::unique_ptr<GameObject>> m_objects;

		// Just a plain-old pointer because the player is a
		// GameObject, hence deleted when the above vector
		// is deleted.
		Player* m_player;

		SDL_Surface *m_name_surf;
		SDL_Surface *m_background_surf;
		SDL_Surface *m_score_surf;

		float m_bonus_counter;
		int m_int_bonus_counter;
		SDL_Surface *m_bonus_surf;
};

struct InGameFactory: public GameLoopFactory
{
	int level;
	int score;

	std::shared_ptr<GameLoop> operator() ();
};

#endif
