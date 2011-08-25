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
#include <memory>

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
#include "GameLoop.hxx"
#include "InGame.hxx"


//
// Implementation
//

// TODO Stop this being a global, it's nasty
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

	Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	SDL_Surface *screen = SDL_SetVideoMode(
		P2_TILE_WIDTH * P2_LEVEL_WIDTH,
		P2_TILE_HEIGHT * P2_LEVEL_HEIGHT,
		24, flags
	);
	SDL_SetEventFilter(event_filter);
	// Did we actually get a hardware, double-buffered surface?
	// If not, it probably isn't vsynced, and we should include
	// a sleep in the main loop
	bool delay = ((screen->flags & flags) != flags);

	//
	// XXX Prototype code XXX
	//

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

	// Create main game loop starting directly from chosen level
	std::shared_ptr<GameLoop> g(new InGame(a, l, level));

	bool quit = false;
	Uint32 frametime = SDL_GetTicks();
	Uint32 old_frametime = frametime;

	while (!quit)
	{
		// Process events
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			// Only quit events are allowed on the queue
			quit = true;
		}

		bool keep = g->update((float)(frametime - old_frametime) / 1000.0,
			SDL_GetKeyState(NULL), screen);

		if (!keep)
		{
			std::unique_ptr<GameLoopFactory> f(g->nextLoop());
			g.reset();

			if (f.get() == 0)
				break;
			else
				g = (*f)();
		}

		SDL_Flip(screen);
		if (delay)
			SDL_Delay(20);

		old_frametime = frametime;
		frametime = SDL_GetTicks();
	}

	return 0;
}
