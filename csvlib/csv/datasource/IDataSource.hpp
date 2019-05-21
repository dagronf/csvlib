//
//  IDataSource.hpp
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

#include <string>
#include <vector>
#include <assert.h>

namespace csv {

class file_exception: public std::exception {
	virtual const char* what() const noexcept {
		return "Unable to open file";
	}
};

class data_exception: public std::exception {
	virtual const char* what() const noexcept {
		return "Unable to set string data for source for the specified encoding";
	}
};

/// Abstract base class for CSV/TSV data sources
class IDataSource {

public:
	virtual ~IDataSource() {}

public:

	/// Remove leading whitespace from fields
	bool trimLeadingWhitespace = true;

	/// Ignore (step over) blank lines rather than return a blank record for empty lines
	bool skipBlankLines = true;

	/// Set to true to cancel the current parsing
	bool cancelled = false;

	/// Move to the next character.  Returns 'false' if EOF is found
	virtual bool next() = 0;
	/// Move back a single character.  If at the start of the data source, then no change.
	virtual void back() = 0;

	/// Is the current character a comment character?
	virtual bool is_comment() const = 0;
	/// Is the current character a separator character?
	virtual bool is_separator() const = 0;
	/// Is the current character a whitespace character?
	virtual bool is_whitespace() const = 0;
	/// Is the current character a double-quote character?
	virtual bool is_quote() const = 0;
	/// Are we at the end of the line?
	virtual bool is_eol() = 0;

	/// Clear the contents of the current field
	virtual void clear_field() = 0;

	/// Push the current character into the current field
	virtual void push() = 0;

	/// Returns the UTF-8 representation of current field
	virtual std::string field() const = 0;

	// Progress through parsing (0.0 -> 1.0)
	virtual double progress() = 0;
};

};
