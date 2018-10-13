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

#include "unicode/ucsdet.h"
#include "unicode/uclean.h"

namespace csv
{
	namespace icu
	{
		std::string DataSource::TextEncodingForData(const char* data, size_t length)
		{
			UErrorCode uerr = U_ZERO_ERROR;
			UCharsetDetector *ucd = ucsdet_open ( &uerr );
			ucsdet_setText(ucd, data, static_cast<int>(length), &uerr);
			UCharsetMatch const * match = ucsdet_detect(ucd, &uerr);
			
			std::string detected = ucsdet_getName(match, &uerr);
			//		printf("Name: %s\n", detected.c_str());
			//		printf("Lang: %s\n", ucsdet_getLanguage(match, &uerr));
			//		printf("Confidence: %u\n", ucsdet_getConfidence(match, &uerr));
			ucsdet_close(ucd);
			u_cleanup(); // keep valgrind happy!!
			
			return detected;
		}
		
		std::string DataSource::TextEncodingForFile(const char* file)
		{
			UErrorCode u_glob_status = U_ZERO_ERROR;
			FILE* fp(0);
			
			u_init(&u_glob_status);
			if (U_SUCCESS(u_glob_status) &&
				(fp = fopen(file, "rb")) != NULL)
			{
				// Read (up to) the first 4k.  It should give us enough to detect the file
				char buf[4096];
				size_t read = fread(buf, 1, 4096, fp);
				
				fclose(fp);
				return TextEncodingForData(buf, read);
			}
			return "";
		}
	};
	
	namespace icu
	{
		bool DataSource::isSeparator()
		{
			return _current == separator;
		}
		
		bool DataSource::isComment()
		{
			return comment != '\0' && comment != _current;
		}
		
		bool DataSource::isWhitespace()
		{
			return _current == ' ';
		}
		
		bool DataSource::isQuote()
		{
			return _current == '\"';
		}
		
		bool DataSource::isEOL()
		{
			if (_current == '\r')
			{
				if (next() == false)
				{
					// Hit the end of file.  Return true and let the caller handle it
					return true;
				}
				
				if (_current != '\n')
				{
					back();
				}
				return true;
			}
			else if (_current == '\n')
			{
				return true;
			}
			return false;
		}
		
		void DataSource::clear_field()
		{
			_field.remove();
		}
		
		void DataSource::push()
		{
			_field += _current;
		}
		
		std::string DataSource::field()
		{
			std::string converted;
			_field.toUTF8String(converted);
			return converted;
		}
	};
	
	namespace icu
	{
		bool FileDataSource::open(const char* file, const char* codepage)
		{
			std::string file_codepage = codepage ?: "";
			if (file_codepage.length() == 0)
			{
				file_codepage = TextEncodingForFile(file);
				if (file_codepage.length() == 0)
				{
					return false;
				}
			}
			
			_in = u_fopen(file, "rb", NULL, file_codepage.c_str());
			return _in != NULL;
		}
		
		FileDataSource::~FileDataSource()
		{
			u_fclose(_in);
		}
		
		bool FileDataSource::next()
		{
			if (u_feof(_in))
			{
				return false;
			}
			
			_prev = _current;
			_current = u_fgetc(_in);
			
			return true;
		}
		
		void FileDataSource::back()
		{
			u_fungetc(_current, _in);
			_current = _prev;
			_prev = 0;
		}
	};
	
	namespace icu
	{
		bool StringDataSource::set(const std::string& text, const char* codepage)
		{
			if (codepage == NULL)
			{
				codepage = TextEncodingForData(text.c_str(), text.length()).c_str();
			}
			
			_in = U_ICU_NAMESPACE::UnicodeString(text.c_str(), codepage);
			return true;
		}
		
		bool StringDataSource::next()
		{
			_offset++;
			if (_offset >= _in.length())
			{
				return false;
			}
			
			_prev = _current;
			_current = _in[_offset];
			return true;
		}
		
		void StringDataSource::back()
		{
			_current = _prev;
			_prev = 0;
			_offset--;
		}
	};
};

#endif
