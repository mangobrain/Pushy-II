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

Alphabet::Alphabet(const char *filename)
{
	// Open glyph set file
	std::ifstream setfile;
	setfile.exceptions(std::ios::badbit | std::ios::failbit);
	setfile.open(filename, std::ios_base::binary);
	
	// Read in 64 glyphs
	// XXX Not sure if 64 glyphs is a hardcoded constant, or whether
	// there's supposed to be some way to tell from the data
	m_glyphs.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
		// Each glyph has an eight byte header - seek to it now
		setfile.seekg(i * 8);

		// Four bytes: offset of glyph from start of file
		// 32-bit, unsigned, little endian
		unsigned char c[4];
		setfile.read((char*)(&c), 4);
		size_t offset = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));

		// Two bytes: unknown
		setfile.read((char*)(&c), 2);

		// Two bytes: glyph height in rows
		// 16-bit, unsigned, little endian
		setfile.read((char*)(&c), 2);
		size_t height = (c[0] | (c[1] << 8));

		// TODO Read in data & store in a surface
	}
}
