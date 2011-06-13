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
		virtual void render(SDL_Surface *screen) = 0;
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
		uint32_t m_anim_x;
		uint32_t m_anim_y;
		uint8_t m_anim_index;
		uint8_t m_anim_state;
		bool arrived(uint8_t x, uint8_t y) const;
		void slideTo(uint8_t x, uint8_t y, uint32_t speed);
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
		void render(SDL_Surface *screen);
	private:
		bool m_rolling;
		uint32_t m_speed;
		uint32_t m_roll_momentum;
};

class Box: public PushableObject
{
	public:
		Box(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		bool rolls() const;
		void push(Direction d);
		void render(SDL_Surface *screen);
};

class Player: public GameObject, AnimableObject
{
	public:
		Player(const TileSet *sprites, const uint8_t *tilemap,
			uint8_t x, uint8_t y, GameObject **objects,
			uint8_t first_floor_tile, uint8_t first_cross_tile);
		void render(SDL_Surface *screen);
		void move(Direction d);
	private:
		uint32_t m_speed;
		bool m_busy;
		uint32_t m_push_momentum;
		bool m_straining;
};

#endif
