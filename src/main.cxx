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

//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language
#include <iostream>
#include <cerrno>
#include <cstring>

// System
#include <unistd.h>

// Library
#include <SDL.h>
#include <getopt.h>

// Local
#include "Constants.hxx"
#include "TileSet.hxx"
#include "LevelSet.hxx"
#include "GameObjects.hxx"

//
// Implementation
//

int event_filter(const SDL_Event *event)
{
	if (event->type == SDL_KEYDOWN || event->type == SDL_QUIT)
		return 1;
	return 0;
}

int main(int argc, char *argv[])
{
	//
	// Command-line option parsing
	//
	
	// Flags for argument-less option presence
	int help = 0;
	int version = 0;

	// Supported command-line options
	struct option long_options[] =
	{
		{"help", no_argument, &help, 'h'},
		{"version", no_argument, &version, 'v'}
	};
	const char optstring[] = "hv";

	// Option parsing loop
	char optchar;
	int optindex;
	while ((optchar =
		getopt_long(argc, argv, optstring, long_options, &optindex)) > -1)
	{
		// Long options without arguments are dealt with for us.
		// For short options, or options with arguments, getopt_long
		// returns the "val" of the option, i.e. the character code
		// of the option's short form.
		switch (optchar)
		{
			case 0:
				// Long option which set a flag
				break;
			case 'h':
				help = 1;
				break;
			case 'v':
				version = 1;
				break;
			default:
				std::cerr << "Unrecognised option" << std::endl;
				return -1;
		}
	}

	// Honour command-line options
	if (help)
	{
		std::cout << PACKAGE_STRING << std::endl << std::endl;
		std::cout << "-h, --help" << std::endl;
		std::cout << "\tPrint this message" << std::endl;
		std::cout << "-v, --version" << std::endl;
		std::cout << "\tDisplay program version and build options" << std::endl;
		return 0;
	}
	else if (version)
	{
		std::cout << PACKAGE_STRING << std::endl << std::endl;
		std::cout << "Built with: " << __P2_CONFIGURE_OPTS << std::endl;
		return 0;
	}

	//
	// Initialise SDL
	//

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cerr << "Could not initialise SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	//
	// Load in resources
	//
	if (chdir(__P2_PKGDATADIR) < 0)
	{
		std::cerr << "Could not change working directory to \""
			<< __P2_PKGDATADIR << "\": " << strerror(errno);
		return 1;
	}
	LevelSet l("LegoLev");
	const TileSet &t(l.getTiles());

	//
	// XXX Prototype code XXX
	//
	#define __TEST_LEVEL 2
	SDL_Surface *screen = SDL_SetVideoMode(
		__TILE_WIDTH * __LEVEL_WIDTH,
		__TILE_HEIGHT * __LEVEL_HEIGHT,
		24, SDL_HWSURFACE | SDL_DOUBLEBUF
	);

	// Create GameObjects array
	// Keep track of them in a vector too, regardless of their
	// position in the game world, so we can render them without
	// iterating over the whole lot
	GameObject *objects[__LEVEL_WIDTH * __LEVEL_HEIGHT];
	std::vector<GameObject*> v_objects;
	Player *p = NULL;
	memset(objects, NULL, sizeof(objects));
	for (uint8_t i = 0; i < l[__TEST_LEVEL].num_sprites; ++i)
	{
		const SpriteInfo *s = &(l[__TEST_LEVEL].spriteinfo[i]);
		GameObject **o = &(objects[(s->y * __LEVEL_WIDTH) + s->x]);
		switch (s->index)
		{
			case 0:
				*o = new Player(&(l.getPlayerSprites()),
					l[__TEST_LEVEL].tilemap, s->x, s->y, objects,
					l.firstFloorTile(), l.firstCrossTile());
				p = (Player*) *o;
				break;
			case 1:
				*o = new Box(&(l.getSprites()),
					l[__TEST_LEVEL].tilemap, s->x, s->y, objects,
					l.firstFloorTile(), l.firstCrossTile());
				break;
			case 2:
				*o = new Ball(&(l.getSprites()),
					l[__TEST_LEVEL].tilemap, s->x, s->y, objects,
					l.firstFloorTile(), l.firstCrossTile());
		}
		v_objects.push_back(*o);
	}

	bool quit = false;
	SDL_SetEventFilter(event_filter);
	while (!quit)
	{
		// Process events
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							quit = true;
							break;
						case SDLK_UP:
							p->move(Up);
							break;
						case SDLK_DOWN:
							p->move(Down);
							break;
						case SDLK_LEFT:
							p->move(Left);
							break;
						case SDLK_RIGHT:
							p->move(Right);
							break;
						default:
							break;
					}
					break;
				case SDL_QUIT:
					quit = true;
			}
		}

		// Render background & game objects
		const uint8_t *tilemap = l[__TEST_LEVEL].tilemap;
		for (int y = 0; y < __LEVEL_HEIGHT; ++y)
		{
			for (int x = 0; x < __LEVEL_WIDTH; ++x)
			{
				SDL_Rect rect = {
					x * __TILE_WIDTH,
					y * __TILE_HEIGHT,
					0, 0
				};
				SDL_BlitSurface(t[tilemap[(y * __LEVEL_WIDTH) + x]],
					NULL, screen, &rect);
			}
		}
		for (std::vector<GameObject*>::const_iterator i = v_objects.begin();
			i != v_objects.end(); ++i)
		{
			(*i)->render(screen);
		}
		SDL_Flip(screen);
		SDL_Delay(40);
	}

	return 0;
}
