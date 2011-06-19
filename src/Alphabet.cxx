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

Alphabet::~Alphabet()
{
	for (std::vector<SDL_Surface*>::iterator i = m_glyphs.begin();
		i != m_glyphs.end(); ++i)
	{
		SDL_FreeSurface(*i);
	}
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
	m_glyph_y_offsets.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
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

		// One byte: unknown
		setfile.read((char*)(c), 1);

		// One byte: glyph Y offset
		// Some glyphs are shorter than others;
		// offset Y coordinate by this much to line up
		setfile.read((char*)(c), 1);
		m_glyph_y_offsets.push_back(c[0]);

		// Two bytes: glyph height in rows
		// 16-bit, unsigned, little endian
		setfile.read((char*)(c), 2);
		uint16_t height = (c[0] | (c[1] << 8));

		// 8bpp (1 byte), so width is simply size / height
		uint16_t width = (uint16_t)(bytes / (size_t)height);

		// Original version appears to render glyphs at double width.
		// Quick hack to make that work: make a double-width surface,
		// and simply plot each colour value twice in a row.
		SDL_Surface *tile = SDL_CreateRGBSurface(SDL_HWSURFACE, width * 2, height, 32,
			0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);

		// Byte value 0x00 seems to be used for transparent
		SDL_SetColorKey(tile, SDL_SRCCOLORKEY, SDL_MapRGB(tile->format, 0x00, 0x00, 0x00));

		// Read in glyph data
		unsigned char * buff = new unsigned char[bytes];
		setfile.seekg(offset);
		setfile.read((char*)(buff), bytes);

		for (int y = 0; y < height; ++y)
		{
			size_t rowstart = y * tile->pitch;
			size_t xoff = 0;
			for (int x = 0; x < width; ++x)
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
				uint8_t val = buff[(y * width) + x];
				bool inside = false;
				if (val > 0)
				{
					if (val < 144)
						val += 112;
					else
						inside = true;
				}
				// Quick hack to double up glyph width.
				for (int twice = 0; twice < 2; ++twice)
				{
					uint32_t *pixel = (uint32_t*)(((char*)tile->pixels) + rowstart + xoff);
					if (inside)
					{
						uint8_t rval = val * 0.2;
						uint8_t gval = val * 0.8;
						uint8_t bval = val;
						*pixel = ((rval >> tile->format->Rloss) << tile->format->Rshift)
							| ((gval >> tile->format->Gloss) << tile->format->Gshift)
							| ((bval >> tile->format->Bloss) << tile->format->Bshift);
					}
					else
					{
						*pixel = ((val >> tile->format->Rloss) << tile->format->Rshift)
							| ((val >> tile->format->Gloss) << tile->format->Gshift)
							| ((val >> tile->format->Bloss) << tile->format->Bshift);
					}
					xoff += tile->format->BytesPerPixel;
				}
			}
		}

		m_glyphs.push_back(tile);
	}
}
