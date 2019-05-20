//
//  ICUDataSource.cpp
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

#include "DataSource.hpp"

#ifdef ALLOW_ICU_EXTENSIONS

#include "Encoding.hpp"

namespace csv {
namespace icu {

	bool DataSource::isSeparator() {
		return _current == separator;
	}

	bool DataSource::isComment() {
		return comment != '\0' && comment != _current;
	}

	bool DataSource::isWhitespace() {
		return _current == ' ';
	}

	bool DataSource::isQuote() {
		return _current == '\"';
	}

	bool DataSource::isEOL() {
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

	void DataSource::clear_field() {
		_field.remove();
	}

	void DataSource::push() {
		_field += _current;
	}

	std::string DataSource::field() {
		std::string converted;
		_field.toUTF8String(converted);
		return converted;
	}
};

namespace icu {
	bool FileDataSource::open(const char* file, const char* codepage) {
		std::string file_codepage = codepage ?: "";
		if (file_codepage.length() == 0) {
			const auto detected = encoding::TextEncodingForFile(file);
			if (detected.invalid()) {
				return false;
			}
			file_codepage = detected.name;
		}

		_in = u_fopen(file, "rb", NULL, file_codepage.c_str());
		if (_in != NULL) {
			FILE* internal_file = u_fgetfile(_in);
			fseek(internal_file, 0L, SEEK_END);
			_length = ftell(internal_file);
			fseek(internal_file, 0L, SEEK_SET);
		}

		return _in != NULL;
	}

	FileDataSource::~FileDataSource() {
		u_fclose(_in);
	}

	bool FileDataSource::next() {
		if (u_feof(_in)) {
			return false;
		}

		_prev = _current;
		_current = u_fgetc(_in);

		return true;
	}

	void FileDataSource::back() {
		u_fungetc(_current, _in);
		_current = _prev;
		_prev = 0;
	}

	double FileDataSource::progress() {
		// ICU appears to perform buffering from the underlying file handle in order to optimize reading
		// As such this method is not byte-accurate as it will depend on how much data ICU buffers during reading
		// So, for small files the progress will always be 1.0, however for larger files it will chunk.
		FILE* internal_file = u_fgetfile(_in);
		double pos = ftell(internal_file);
		double len = _length;
		return std::min(pos / len, 1.0);
	}
};

namespace icu {

	bool StringDataSource::set(const std::string& text, const char* codepage) {
		std::string cp = codepage ?: "";
		if (codepage == NULL) {
			const auto detected = encoding::TextEncodingForData(text.c_str(), text.length());
			if (detected.invalid()) {
				return false;
			}
			cp = detected.name;
		}

		_in = U_ICU_NAMESPACE::UnicodeString(text.c_str(), (int)(text.length()), cp.c_str());
		return true;
	}

	bool StringDataSource::next() {
		_offset++;
		if (_offset >= _in.length()) {
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

	double StringDataSource::progress() {
		double pos = _offset;
		double len = _in.length();
		return std::min(pos / len, 1.0);
	}
};
};

#endif
