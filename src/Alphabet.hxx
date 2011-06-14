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

#include <vector>

// Class for rendering coloured strings
// composed of glyphs loaded from the Alphabet file
//
// TODO Method for rendering a glyph with a selected
// RGB value, for coloured level names.  Maybe storing
// glyphs pre-converted to SDL surfaces isn't the way
// to go?
class Alphabet
{
	public:
		Alphabet(const char *filename);
		~Alphabet();

		SDL_Surface *operator[](std::vector<SDL_Surface*>::size_type index) const
		{
			return m_glyphs[index];
		};

		uint8_t getYOffset(std::vector<uint8_t>::size_type index) const
		{
			return m_glyph_y_offsets[index];
		};

		std::vector<SDL_Surface*>::size_type numGlyphs() const
		{
			return m_glyphs.size();
		};

	private:
		std::vector<SDL_Surface*> m_glyphs;
		std::vector<uint8_t> m_glyph_y_offsets;
};

#endif
