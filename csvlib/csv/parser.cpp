//
//  CSVParser.cpp
//  csv_parser_test
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

#include <iostream>

#include "parser.hpp"

#define RETURN_IF_CANCELLED(parser) 	if (parser.cancelled) { return InternalState::Canceled; }

namespace {

	/// Internal parser state
	typedef enum InternalState {
		EndOfField = 0,
		EndOfLine = 1,
		EndOfFile = 2,
		Canceled = 3
	} InternalState;

	bool parseSeparator(csv::IDataSource& parser) {
		if (parser.is_separator()) {
			// If a separator, then move to the next character
			return parser.next();
		}
		return false;
	}

	void skipWhitespace(csv::IDataSource& parser) {
		while (parser.is_whitespace() && parser.next()) {
			// Just continue reading.
		}
	}

	InternalState parseEscapedString(csv::IDataSource& parser) {
		// escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE

		// Quote has already been read.  Move to the next char
		if (!parser.next()) {
			return InternalState::EndOfFile;
		}

		while (true) {

			RETURN_IF_CANCELLED(parser);

			if (parser.is_quote()) {
				if (!parser.next()) {
					return InternalState::EndOfFile;
				}
				else if (parser.is_quote()) {
					// 2DQUOTE -- push the quote into the field.
					parser.push();
				}
				else {
					// If we've hit the end of an escaped string, we should attempt to locate either
					// 1. The next separator
					// 2. End of line
					while (true) {
						if (parser.is_eol()) {
							return InternalState::EndOfLine;
						}

						if (parser.is_separator()) {
							return InternalState::EndOfField;
						}

						if (!parser.next()) {
							return InternalState::EndOfFile;
						}
					}
				}
			}
			else {
				parser.push();
			}

			if (!parser.next()) {
				return InternalState::EndOfFile;
			}
		}
		return InternalState::EndOfField;
	}

	InternalState parseUnescapedString(csv::IDataSource& parser) {
		// non-escaped = *TEXTDATA
		while (true) {

			RETURN_IF_CANCELLED(parser);

			if (parser.is_separator()) {
				return InternalState::EndOfField;
			}
			else if (parser.is_eol()) {
				return InternalState::EndOfLine;
			}
			else if (parser.is_quote()) {
				if (!parser.next()) {
					return InternalState::EndOfFile;
				}

				if (parser.is_quote()) {
					// Double quote. This is fine.
					parser.push();
				}
				else {
					// This is an error case.  A single quote in an unescaped
					// string is bad.  Lets try to recover (assume single quote)
					parser.back();
					parser.push();
				}
			}
			else {
				parser.push();
			}

			// Move to the next character
			if (!parser.next()) {
				return InternalState::EndOfFile;
			}
		}
		return InternalState::EndOfField;
	}

	InternalState parseField(csv::IDataSource& parser, bool isFirstFieldForRow) {
		//  field = (escaped / non-escaped)

		parser.clear_field();

		// If the first character on the line is a comment character, then
		// skip the line completely.  Only support for single line comments
		if (isFirstFieldForRow && parser.is_comment()) {
			// Skip to end of the line
			while (!parser.is_eol()) {
				if (!parser.next()) {
					return InternalState::EndOfFile;
				}
			}
			return InternalState::EndOfLine;
		}

		if (parser.trimLeadingWhitespace) {
			skipWhitespace(parser);
		}

		if (parser.is_eol()) {
			// Had whitespace before the end of the line, so treat it as
			// a blank field
			return InternalState::EndOfLine;
		}

		InternalState returnState = InternalState::EndOfField;
		if (parser.is_quote()) {
			returnState = parseEscapedString(parser);
		}
		else {
			returnState = parseUnescapedString(parser);
		}

		return returnState;
	}

	InternalState parseRecord(csv::IDataSource& parser,
							  csv::record& record,
							  csv::FieldCallback emitField) {

		//  record = field *(COMMA field)

		csv::field field;

		InternalState state = InternalState::EndOfField;
		bool isNewRecord = true;
		size_t column = 0;

		while (true) {
			RETURN_IF_CANCELLED(parser);
			state = parseField(parser, isNewRecord);
			RETURN_IF_CANCELLED(parser);

			field.content = parser.field();
			field.column = column;
			field.row = record.row;

			record.add(field);
			if (emitField && emitField(field) == false) {
				return InternalState::EndOfFile;
			}

			isNewRecord = false;

			switch (state) {
				case InternalState::EndOfField:
					if (!parseSeparator(parser)) {
						// We have a separator at the last character in a file, which means an
						// empty field right at the end.
						field.content = "";
						field.column = column + 1;
						field.row = record.row;
						record.add(field);
						return InternalState::EndOfFile;
					}
					break;
				default:
					// We have finished the current record
					return state;
			}
			column++;
		}
	}
};

namespace csv {

	State parse(IDataSource& parser, FieldCallback emitField, RecordCallback emitRecord) {
		//  file = [header CRLF] record *(CRLF record) [CRLF]

		InternalState state = InternalState::EndOfFile;
		parser.cancelled = false;

		// Move to the first character
		if (!parser.next()) {
			// File is empty.  Do nothing
			return State::Complete;
		}

		csv::record record;

		size_t row = 0;
		do {
			record.content.clear();
			record.row = row;

			state = parseRecord(parser, record, emitField);

			if (!parser.skipBlankLines || !record.empty()) {
				row++;
				if (emitRecord && (emitRecord(record, parser.progress()) == false)) {
					return State::Complete;
				}
			}

			if (!parser.next()) {
				return State::Complete;
			}
		}
		while (state != InternalState::Canceled && state != InternalState::EndOfFile);

		switch (state) {
			case InternalState::Canceled:
				return State::Cancelled;
			case InternalState::EndOfFile:
				return State::Complete;
			default:
				assert(false);
				return State::Error;
		}
	}
};
