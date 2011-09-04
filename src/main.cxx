// Copyright 2010-2011 Philip Allison

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
#include <getopt.h>

// Local
#include "MainMenu.hxx"


//
// Implementation
//

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
	SDL_WM_SetCaption("Pushy II", "Pushy II");
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetEventFilter(event_filter);
	SDL_Surface *screen = SDL_SetVideoMode(
		P2_TILE_WIDTH * P2_LEVEL_WIDTH,
		P2_TILE_HEIGHT * P2_LEVEL_HEIGHT,
		24, flags
	);

	// Did we actually get a hardware, double-buffered surface?
	// If not, it probably isn't vsynced, and we should include
	// a sleep in the main loop
	bool delay = ((screen->flags & flags) != flags);

	// Create main menu loop
	std::shared_ptr<GameLoop> g(new MainMenu(a, l));

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

		// Update state & render current frame
		bool keep = g->update((float)(frametime - old_frametime) / 1000.0f,
			SDL_GetKeyState(NULL), screen);

		// If the current GameLoop should not be kept,
		// construct the next one, or exit
		if (!keep)
		{
			std::unique_ptr<GameLoopFactory> f(g->nextLoop());
			g.reset();

			if (f.get() == 0)
				break;
			else
			{
				g = (*f)();

				// Quick & dirty transition between the last
				// frame from the old GameLoop & the first frame
				// from the new one
				SDL_Surface *old_sf = SDL_DisplayFormat(screen);
				SDL_Surface *new_sf = SDL_DisplayFormat(screen);
				g->update(0.0f, SDL_GetKeyState(NULL), new_sf);
				float y = 0.0f;
				frametime = SDL_GetTicks();
				old_frametime = frametime;
				while ((y - (float)P2_TILE_HEIGHT)
					< (float)(P2_TILE_HEIGHT * P2_LEVEL_HEIGHT))
				{
					// Start by rendering the destination
					SDL_BlitSurface(new_sf, NULL, screen, NULL);

					// Move the bottom of the vertical wipe down
					// based on time elapsed since last frame
					y += 512.0f * ((float)(frametime - old_frametime) / 1000.0f);

					// Top of the vertical wipe is two tile
					// heights above the bottom - but not off-screen
					float i = y - (float)(P2_TILE_HEIGHT * 2);
					if (i < 0.0f)
						i = 0.0f;

					// Vertical wipe by rendering a series of
					// single rows each 1 pixel high
					for (; i <= y; ++i)
					{
						if ((Sint16)i >= (P2_TILE_HEIGHT * P2_LEVEL_HEIGHT))
							break;

						SDL_Rect src = {
							0, (Sint16)i, P2_TILE_WIDTH * P2_LEVEL_WIDTH, 1
						};
						SDL_Rect dst = {
							0, (Sint16)i, 0, 0
						};

						// Transparency ranges from 0 to 255 over the height
						// of the wipe (2 tiles)
						SDL_SetAlpha(old_sf, SDL_SRCALPHA,
							255 - (Uint8)(((y - i) / (float)(P2_TILE_HEIGHT * 2)) * 255.0f));

						SDL_BlitSurface(old_sf, &src, screen, &dst);
					}

					// Render the rest of the start surface
					// opaque below the wipe
					if (y < (float)(P2_TILE_HEIGHT * P2_LEVEL_HEIGHT))
					{
						SDL_SetAlpha(old_sf, 0, 0);
						SDL_Rect src = {
							0, (Sint16)y, P2_TILE_WIDTH * P2_LEVEL_WIDTH,
							(Uint16)(old_sf->h - (int)y)
						};
						SDL_Rect dst = {
							0, (Sint16)y, 0, 0
						};
						SDL_BlitSurface(old_sf, &src, screen, &dst);
					}

					SDL_Flip(screen);
					if (delay)
						SDL_Delay(20);

					old_frametime = frametime;
					frametime = SDL_GetTicks();
				}

				SDL_FreeSurface(old_sf);
				SDL_FreeSurface(new_sf);

				// Don't jump game state ahead by the time taken
				// to perform the transition
				frametime = SDL_GetTicks();
			}
		}

		SDL_Flip(screen);
		if (delay)
			SDL_Delay(20);

		old_frametime = frametime;
		frametime = SDL_GetTicks();
	}

	return 0;
}
