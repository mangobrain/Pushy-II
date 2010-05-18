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

#ifndef __HXX_GAMEOBJECTS

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

		const TileSet *_sprites;
		uint8_t _x;
		uint8_t _y;
		GameObject **_objects;

	private:
		const uint8_t *_tilemap;
		const uint8_t _first_floor_tile;
		const uint8_t _first_cross_tile;
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
		uint32_t _anim_x;
		uint32_t _anim_y;
		uint8_t _anim_index;
		uint8_t _anim_state;
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
};

#endif
