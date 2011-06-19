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

//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language
#include <fstream>
#include <stdexcept>
#include <new>
#include <memory>

// System

// Library
#include <SDL.h>

// Local
#include "Alphabet.hxx"

//
// Implementation
//

Alphabet::Glyph::Glyph()
	: values(NULL)
{
}

Alphabet::Glyph::~Glyph()
{
	delete[] values;
}

Alphabet::Alphabet(const char *filename)
{
	// Open glyph set file - at end, so we can determine size
	std::ifstream setfile;
	setfile.exceptions(std::ios::badbit | std::ios::failbit);
	setfile.open(filename, std::ios_base::binary | std::ios_base::ate);
	size_t filesize = setfile.tellg();

	// Read in 64 glyphs
	// XXX Not sure if 64 glyphs is a hardcoded constant, or whether
	// there's supposed to be some way to tell from the data
	setfile.seekg(0);
	m_glyphs.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
		m_glyphs.push_back(Glyph());

		// Each glyph has an eight byte header - seek to it now
		setfile.seekg(i * 8);

		// Four bytes: offset of glyph from start of file
		// 32-bit, unsigned, little endian
		unsigned char c[4];
		setfile.read((char*)(c), 4);
		size_t offset = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));

		// Work out glyph size in bytes by comparing either to the offset of
		// the next glyph, or by the known position of EOF
		size_t bytes;
		if (i < 63)
		{
			// Seek to next glyph header; read in offset
			std::ofstream::pos_type saved_pos = setfile.tellg();
			setfile.seekg(4, std::ios_base::cur);
			setfile.read((char*)(c), 4);
			size_t next_offset = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
			bytes = next_offset - offset;
			setfile.seekg(saved_pos);
		}
		else
			bytes = filesize - offset;

		// One byte: glyph X offset
		// Glyphs are not rendered edge-to-edge horizontally,
		// but offset the X position of the next glyph by an
		// amount sometimes subtly different from their actual
		// width - a primitive form of kerning
		// Rember X offset values are already doubled
		setfile.read((char*)(&(m_glyphs.back().x_offset)), 1);

		// One byte: glyph Y offset
		// Some glyphs are shorter than others;
		// offset Y coordinate by this much to line up
		// instead of storing blank rows in the file
		setfile.read((char*)(&(m_glyphs.back().y_offset)), 1);

		// Two bytes: glyph height in rows
		// 16-bit, unsigned, little endian
		setfile.read((char*)(c), 2);
		m_glyphs.back().height = (c[0] | (c[1] << 8));

		// 8bpp (1 byte), so width is simply size / height
		m_glyphs.back().width = bytes / (size_t)(m_glyphs.back().height);

		// Read in glyph data
		m_glyphs.back().values = new uint8_t[bytes];
		setfile.seekg(offset);
		setfile.read((char*)(m_glyphs.back().values), bytes);
	}
}

SDL_Surface * Alphabet::renderWord(const std::string &word,
	unsigned char r, unsigned char g, unsigned char b) const
{
	int indices[word.length()];
	int height = 0;
	int width = 0;
	float fr = (float)(r) / 255.0f;
	float fg = (float)(g) / 255.0f;
	float fb = (float)(b) / 255.0f;
	for (size_t i = 0; i < word.length(); ++i)
	{
		// Figure out glyph index for ASCII character
		if (word[i] >= 'A' && word[i] <= 'Z')
			indices[i] = word[i] - 'A';
		else if (word[i] >= 'a' && word[i] <= 'z')
			indices[i] = (word[i] - 'a') + 26;
		else if (word[i] >= '0' && word[i] <= '9')
			indices[i] = (word[i] - '0') + 52;
		else if (word[i] == ':')
			indices[i] = 62;
		else
			// Render any other unrecognised character as a hyphen
			indices[i] = 63;

		// Store the tallest character to work out the height
		// of surface we'll need to allocate (height + Y offset)
		const Glyph & g(m_glyphs[indices[i]]);
		if (g.height + g.y_offset > height)
			height = g.height + g.y_offset;

		// Add up total width of glyphs in string to get
		// final width of surface.  For glyphs except the
		// last, use the X offset (kerning) instead of
		// the width of the glyph itself.
		// Rember to double up glyph width, but that
		// X offset values are pre-doubled.
		width += (i < (word.length() - 1)) ? g.x_offset : (g.width * 2);
	}

	// Create destination surface
	// 0x00 seems to be used as a colour key in the
	// original data, so mirror that here
	SDL_Surface * surf = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32,
		0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
	SDL_SetColorKey(surf, SDL_SRCCOLORKEY, SDL_MapRGB(surf->format, 0x00, 0x00, 0x00));

	// Render glyphs onto destination surface
	int glyph_horz_start = 0;
	for (size_t i = 0; i < word.length(); ++i)
	{
		const Glyph & g(m_glyphs[indices[i]]);
		for (int y = 0; y < g.height; ++y)
		{
			size_t rowstart = (y + g.y_offset) * surf->pitch;
			size_t xoff = glyph_horz_start * surf->format->BytesPerPixel;
			for (int x = 0; x < g.width; ++x)
			{
				// XXX
				// Glyphs are rendered a shaded greyscale outline,
				// with a coloured inside.  This appears to be encoded
				// in the original 8bpp data by nothing more than
				// thresholding: intensities below a certain value are
				// kept in greyscale, but artificially brightened to
				// top out at 255 instead of the threshold value;
				// intensities above are multiplied by the target
				// RGB values (divided by 255 to give floats in the
				// range 0..1).
				// Experimentation seems to show that intensity
				// values of 144 and above represent the inside.
				uint8_t val = g.values[(y * g.width) + x];
				// Ignore intensity values of 0 completely, so that
				// glyphs don't blat the right-hand edge of the glyph
				// to their left with transparency (they can overlap
				// due to kerning).
				if (val > 0)
				{
					bool inside = false;
					if (val < 144)
						val += 112;
					else
						inside = true;
					// Quick hack to double up column width.
					for (int twice = 0; twice < 2; ++twice)
					{
						uint32_t *pixel = (uint32_t*)(((char*)surf->pixels) + rowstart + xoff);
						if (inside)
						{
							uint8_t rval = val * fr;
							uint8_t gval = val * fg;
							uint8_t bval = val * fb;
							*pixel = ((rval >> surf->format->Rloss) << surf->format->Rshift)
								| ((gval >> surf->format->Gloss) << surf->format->Gshift)
								| ((bval >> surf->format->Bloss) << surf->format->Bshift);
						}
						else
						{
							*pixel = ((val >> surf->format->Rloss) << surf->format->Rshift)
								| ((val >> surf->format->Gloss) << surf->format->Gshift)
								| ((val >> surf->format->Bloss) << surf->format->Bshift);
						}
						xoff += surf->format->BytesPerPixel;
					} // render value twice in X
				}
				else
				{
					// However, since we work with pixel pointer arithmetic,
					// we must still increase our X offset to compensate.
					// Remember to double up on column width.
					xoff += surf->format->BytesPerPixel * 2;
				}
			} // loop over X
		} // loop over Y

		// Offset start of next glyph by X offset of this one
		// Rember X offset values are already doubled
		glyph_horz_start += g.x_offset;
	}

	return surf;
}
