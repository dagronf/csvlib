//
//  CSVParser.hpp
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

#include <csv/datasource/IDataSource.hpp>

#include <functional>

namespace csv
{
	typedef enum State
	{
		Complete = 0,
		Cancelled = 1,
		Error = 2,
	} State;

	struct field
	{
		size_t row = 0;
		size_t column = 0;
		std::string content;
	};

	struct record
	{
		size_t row = 0;
		std::vector<field> content;

		inline size_t size() const { return content.size(); }
		inline const field& operator[](const size_t offset) const { return content[offset]; }

		inline void clear() { content.clear(); }
		inline void add(const field& field) { content.push_back(field); }

		inline bool empty() const
		{
			for (const auto& field: content)
			{
				if (field.content.length() > 0)
				{
					return false;
				}
			}
			return true;
		}
	};

	typedef std::function<bool(const field&)> FieldCallback;
	typedef std::function<bool(const record&)> RecordCallback;
	
	csv::State parse(IDataSource& parser,
					 csv::FieldCallback emitField,
					 csv::RecordCallback emitRecord);
};
