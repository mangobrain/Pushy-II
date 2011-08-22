// Copyright 2010 Philip Allison

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

#ifndef HXX_GAMEOBJECTS

class GameObject
{
	public:
		GameObject(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		virtual void render(SDL_Surface *screen, float elapsed) = 0;
		virtual ~GameObject() {};

	protected:
		bool tileIsEmpty(uint8_t x, uint8_t y) const;
		bool tileIsCross(uint8_t x, uint8_t y) const;

		const TileSet *m_sprites;
		uint8_t m_x;
		uint8_t m_y;
		GameObject **m_objects;

	private:
		const uint8_t *m_tilemap;
		const uint8_t m_first_floor_tile;
		const uint8_t m_first_cross_tile;
};

enum Direction
{
	Left,
	Right,
	Up,
	Down
};

class AnimableObject
{
	public:
		AnimableObject(uint8_t ax, uint8_t ay);
		virtual ~AnimableObject() {};
	protected:
		float m_anim_x;
		float m_anim_y;
		float m_anim_fps;
		uint8_t m_anim_index;
		uint8_t m_anim_state;

		// Move object to given destination square at given speed.
		// Returns true when arrived.
		bool slideTo(uint8_t x, uint8_t y, float speed, float elapsed);

		// Return number of frames by which to advance animation
		int advanceAnim(float elapsed);

	private:
		float m_anim_frames_elapsed;
};

class PushableObject: public GameObject, public AnimableObject
{
	public:
		PushableObject(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		bool canMove(Direction d) const;
		virtual bool rolls() const = 0;
		virtual void push(Direction d) = 0;
		virtual ~PushableObject() {};
	protected:
		bool m_defused;
};

class Ball: public PushableObject
{
	public:
		Ball(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		bool rolls() const;
		void push(Direction d);
		void render(SDL_Surface *screen, float elapsed);
	private:
		bool m_rolling;
		float m_speed;
		float m_roll_momentum;
};

class Box: public PushableObject
{
	public:
		Box(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		bool rolls() const;
		void push(Direction d);
		void render(SDL_Surface *screen, float elapsed);
};

class Player: public GameObject, AnimableObject
{
	public:
		Player(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		void render(SDL_Surface *screen, float elapsed);
		void move(Direction d);
	private:
		float m_speed;
		bool m_busy;
		float m_push_momentum;
		bool m_straining;
};

#endif
