//
//  UTF8DataSource.hpp
//
//  Created by Darren Ford on 7/10/18.
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

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <csv/datasource/IDataSource.hpp>

namespace csv {
namespace utf8 {

class DataSource: public csv::IDataSource {
public:
	DataSource() noexcept;

	char separator = ',';
	char comment = '\0';

protected:

	// Character detection
	
	inline bool isSeparator() {
		return _current == separator;
	}
	inline bool isComment() {
		return comment != '\0' && _current == comment;
	}
	inline bool isWhitespace() {
		return _current == ' ';
	}
	inline bool isQuote() {
		return _current == '\"';
	}

	// Field related

	inline void clear_field() {
		_field.clear();
	}
	inline std::string field() {
		return _field;
	}
	inline void push() {
		_field += _current;
	}

	bool isEOL();

protected:
	char _prev = 0;
	char _current;

private:
	std::string _field;
};

class FileDataSource: public utf8::DataSource {
public:
	FileDataSource() noexcept {}
	FileDataSource(const char* file) {
		if (!open(file)) {
			throw csv::file_exception();
		}
	}
	bool open(const char* file);

public:
	virtual bool next();
	virtual void back();
	virtual double progress();

private:
	std::ifstream _in;
	std::streamsize _length;
};

class StringDataSource: public utf8::DataSource {
public:
	StringDataSource() noexcept : _offset(-1) {}
	StringDataSource(const std::string& data) {
		if (!set(data)) {
			throw csv::data_exception();
		}
	}
	bool set(const std::string& data);

public:
	virtual bool next();
	virtual void back();
	virtual double progress();

private:
	size_t _offset;
	std::string _in;
};

};
};
