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
#include <cstdlib>

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
#include "Alphabet.hxx"

//
// Implementation
//

int objects_left = 0;

int event_filter(const SDL_Event *event)
{
	if (event->type == SDL_QUIT)
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
		std::cout << "Built with: " << P2_CONFIGURE_OPTS << std::endl;
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
	atexit(SDL_Quit);

	//
	// Load in resources
	//
	if (chdir(P2_PKGDATADIR) < 0)
	{
		std::cerr << "Could not change working directory to \""
			<< P2_PKGDATADIR << "\": " << strerror(errno);
		return 1;
	}
	Alphabet a("Alphabet");
	LevelSet l("LegoLev");
	const TileSet &t(l.getTiles());

	//
	// XXX Prototype code XXX
	//
	Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	SDL_Surface *screen = SDL_SetVideoMode(
		P2_TILE_WIDTH * P2_LEVEL_WIDTH,
		P2_TILE_HEIGHT * P2_LEVEL_HEIGHT,
		24, flags
	);
	bool quit = false;
	SDL_SetEventFilter(event_filter);
	// Did we actually get a hardware, double-buffered surface?
	// If not, it probably isn't vsynced, and we should include
	// a sleep in the main loop
	bool delay = ((screen->flags & flags) != flags);

	// Accept passwords on the command line
	size_t level = 0;
	if (optind < argc)
	{
		std::string password = argv[optind];
		for (size_t i = 0; i < l.size(); ++i)
		{
			if (l[level].name == password)
				break;
			++level;
		}
		if (level == l.size())
			level = 0;
	}

	while (!quit && (level < l.size()))
	{
		// Render level name into a surface
		SDL_Surface * name = a.renderWord(l[level].name,
			l[level].name_colour[0],
			l[level].name_colour[1],
			l[level].name_colour[2]);

		// Create GameObjects array
		// Keep track of them in a vector too, regardless of their
		// position in the game world, so we can render them without
		// iterating over the whole lot
		GameObject *objects[P2_LEVEL_WIDTH * P2_LEVEL_HEIGHT];
		std::vector<GameObject*> v_objects;
		Player *p = NULL;
		memset(objects, 0, sizeof(objects));
		objects_left = 0;
		for (uint8_t i = 0; i < l[level].num_sprites; ++i)
		{
			const SpriteInfo *s = &(l[level].spriteinfo[i]);
			GameObject **o = &(objects[(s->y * P2_LEVEL_WIDTH) + s->x]);
			switch (s->index)
			{
				case 0:
					*o = new Player(&(l.getPlayerSprites()),
						l[level].tilemap, s->x, s->y, objects,
						l.firstFloorTile(), l.firstCrossTile());
					p = (Player*) *o;
					break;
				case 1:
					*o = new Box(&(l.getSprites()),
						l[level].tilemap, s->x, s->y, objects,
						l.firstFloorTile(), l.firstCrossTile());
					break;
				case 2:
					*o = new Ball(&(l.getSprites()),
						l[level].tilemap, s->x, s->y, objects,
						l.firstFloorTile(), l.firstCrossTile());
			}
			v_objects.push_back(*o);
			++objects_left;
		}
		// One object is the player
		--objects_left;

		Uint32 frametime = SDL_GetTicks();
		Uint32 old_frametime = frametime;

		while (!quit && objects_left > 0)
		{
			// Process events
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				// Only quit events are allowed on the queue
				quit = true;
			}

			// Handle keypresses separately
			// (we don't care about explicit presses/releases,
			// just which keys are being held down)
			Uint8 *keys = SDL_GetKeyState(NULL);
			if (keys[SDLK_UP])
				p->move(Up);
			else if (keys[SDLK_DOWN])
				p->move(Down);
			else if (keys[SDLK_LEFT])
				p->move(Left);
			else if (keys[SDLK_RIGHT])
				p->move(Right);
			if (keys[SDLK_ESCAPE])
				quit = true;

			// Render background & game objects
			const uint8_t *tilemap = l[level].tilemap;
			for (int y = 0; y < P2_LEVEL_HEIGHT; ++y)
			{
				for (int x = 0; x < P2_LEVEL_WIDTH; ++x)
				{
					SDL_Rect rect = {
						(Sint16)(x * P2_TILE_WIDTH),
						(Sint16)(y * P2_TILE_HEIGHT),
						0, 0
					};
					SDL_BlitSurface(t[tilemap[(y * P2_LEVEL_WIDTH) + x]],
						NULL, screen, &rect);
				}
			}
			for (std::vector<GameObject*>::const_iterator i = v_objects.begin();
				i != v_objects.end(); ++i)
			{
				(*i)->render(screen, (float)(frametime - old_frametime) / 1000.0);
			}

			// Render level name
			SDL_Rect rect = {
				50, 320, 0, 0
			};
			SDL_BlitSurface(name, NULL, screen, &rect);

			SDL_Flip(screen);
			if (delay)
				SDL_Delay(10);

			old_frametime = frametime;
			frametime = SDL_GetTicks();
		}
		++level;
		SDL_FreeSurface(name);
	}

	return 0;
}
