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

#ifndef HXX_ALPHABET
#define HXX_ALPHABET

#include <cstdint>
#include <vector>
#include <string>

#include <SDL.h>

// Class for rendering coloured strings
// composed of glyphs loaded from the Alphabet file.
class Alphabet
{
	public:
		Alphabet(const char *filename);

		// Render a word onto a surface.  ASCII values without a corresponding
		// glyph will be rendered as a hyphen.
		//
		// XXX NB: The returned surface is dynamically allocated, and must
		// be freed by the caller using SDL_FreeSurface.
		SDL_Surface *renderWord(const std::string &word,
			unsigned char r = 255, unsigned char g = 255, unsigned char b = 255) const;

	private:
		struct Glyph
		{
			uint16_t width;
			uint16_t height;
			uint8_t x_offset;
			uint8_t y_offset;
			uint8_t * values;

			Glyph();
			~Glyph();
		};

		// We can get away with this storing Glyphs directly,
		// rather than glyph pointers, because there is a fixed
		// number of glyphs.  Reserving space ahead of time
		// alleviates the need for assignment/copying of Glyphs.
		std::vector<Glyph> m_glyphs;
};

#endif
