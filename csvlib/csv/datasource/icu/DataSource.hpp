//
//  ICUDataSource.hpp
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

#ifdef ALLOW_ICU_EXTENSIONS

#include <csv/datasource/IDataSource.hpp>

#include "unicode/unistr.h"
#include "unicode/ustdio.h"

namespace csv {
namespace icu {

	class DataSource: public csv::IDataSource {
	public:
		virtual ~DataSource() {}

		UChar32 separator = ',';
		UChar32 comment = '\0';

	protected:
		bool isSeparator();
		bool isComment();
		bool isWhitespace();
		bool isQuote();
		bool isEOL();

		void clear_field();
		void push();
		std::string field();

	protected:
		UChar32 _prev = 0;
		UChar32 _current;

	private:
		U_ICU_NAMESPACE::UnicodeString _field;
	};

	class FileDataSource: public DataSource {
	public:
		FileDataSource() noexcept {}
		FileDataSource(const std::string& file, const char* codepage) {
			if (!open(file.c_str(), codepage)) {
				throw std::runtime_error("Unable to open file for source");
			}
		}

		bool open(const char* file, const char* codepage);
		virtual ~FileDataSource();

	protected:
		bool next();
		void back();

	private:
		UFILE* _in;
	};

	class StringDataSource: public DataSource {
	public:
		StringDataSource() noexcept : _offset(-1) {}
		StringDataSource(const std::string& data, const char* codepage) {
			if (!set(data, codepage)) {
				throw std::runtime_error("Unable to set string data for source");
			}
		}

		bool set(const std::string& text, const char* codepage);

	protected:
		bool next();
		void back();

	private:
		U_ICU_NAMESPACE::UnicodeString _in;
		int32_t _offset = -1;
	};

};
};

#endif
