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

namespace csv
{
	class IDataSource
	{
	public:
		bool trimLeadingWhitespace = true;
		bool keepBlankLines = false;
		
		bool cancelled = false;

		/// Move to the next character.  Returns 'false' if EOF is found
		virtual bool next() = 0;

		/// Move back a single character
		virtual void back() = 0;

		virtual bool isComment() = 0;
		virtual bool isSeparator() = 0;
		virtual bool isWhitespace() = 0;
		virtual bool isQuote() = 0;
		virtual bool isEOL() = 0;

		virtual void clear_field() = 0;
		virtual void push() = 0;
		virtual std::string field() = 0;
	};
};
