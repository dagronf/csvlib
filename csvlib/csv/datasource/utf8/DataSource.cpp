//
//  UTF8CSVParser.cpp
//  csv_parser_test
//
//  Created by Darren Ford on 8/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//
//  MIT license
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <stdio.h>
#include <string.h>
#include <limits>
#include <array>

#include "DataSource.hpp"

// MARK: - UTF8 data source

namespace csv {
namespace utf8 {

	// BOM definitions
	static const std::string _BOMS = { '\xEF', '\xBB', '\xBF' };
	static const size_t _BOMS_SIZE = 3;

	DataSource::DataSource() noexcept {
		_field.reserve(256);
	}

	bool DataSource::is_eol() {
		if (_current == '\r') {
			if (next() == false) {
				// Hit the end of file.  Return true and let the caller handle it
				return true;
			}

			if (_current != '\n') {
				back();
			}
			return true;
		}
		else if (_current == '\n') {
			return true;
		}
		return false;
	}
};
};

// MARK: - UTF8 file source

namespace csv {
namespace utf8 {

	FileDataSource::~FileDataSource() {
		close();
	}

	void FileDataSource::close() {
		if (_in.is_open()) {
			_in.close();
		}
	}

	bool FileDataSource::open(const char* file) {

		// If we have one open, close it first
		close();

		_in.open(file, std::ios::in | std::ios::binary);
		if (_in.is_open()) {

			// Get the size
			_in.ignore( std::numeric_limits<std::streamsize>::max() );
			_length = _in.gcount();
			_in.clear();   //  Since ignore will have set eof.
			_in.seekg( 0, std::ios_base::beg );

			if (_length < _BOMS_SIZE) {
				// If we have less chars in the file than the size of the BOM.
				return true;
			}

			// Check if we have a BOM - Read the first three bytes.
			std::array<char, _BOMS_SIZE> utf8BOM;
			std::fill(utf8BOM.begin(), utf8BOM.end(), 0);
			_in.read(utf8BOM.data(), utf8BOM.size());

			if (memcmp(_BOMS.c_str(), utf8BOM.data(), _BOMS_SIZE) != 0) {
				_in.clear();					// clear fail and eof bits
				_in.seekg(0, std::ios::beg);	// back to the start!
			}
		}

		return _in.is_open();
	}

	double FileDataSource::progress() {
		double pos = _in.tellg();
		double len = _length;
		return std::min(pos / len, 1.0);
	}

	bool FileDataSource::next() {
		if (_in.peek() == EOF) {
			return false;
		}

		_prev = _current;
		_in.get(_current);
		return true;
	}

	void FileDataSource::back() {
		_current = _prev;
		_prev = 0;
		_in.unget();
	}
};

// MARK: - UTF8 string source

namespace utf8 {

	bool StringDataSource::set(const std::string& data) {
		_in = data;
		_offset = -1;
		_prev = 0;

		// Check for a BOM and skip it
		if (data.length() < _BOMS.size()) {
			// Cannot possibly have a BOM
			return true;
		}

		if (memcmp(data.c_str(), _BOMS.c_str(), _BOMS.size()) == 0) {
			// We have a BOM. Set the starting offset to AFTER it
			// (note that the string offset starts at -1, so we need to set offset to 3 - 1)
			_offset = 2;
		}

		return true;
	}

	double StringDataSource::progress() {
		double pos = _offset;
		double len = _in.length();
		return std::min(pos / len, 1.0);
	}

	bool StringDataSource::next() {
		_offset++;
		if (_offset >= _in.size()) {
			return false;
		}

		_prev = _current;
		_current = _in[_offset];
		return true;
	}

	void StringDataSource::back() {
		_current = _prev;
		_prev = 0;
		_offset--;
	}
};
};
