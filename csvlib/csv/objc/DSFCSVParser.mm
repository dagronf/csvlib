//
//  DSFCSVParser.mm
//  csv_parser_test
//
//  Created by Darren Ford on 9/10/18.
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

// Only for compiling with objective-c
#ifdef __OBJC__

#import "DSFCSVParser.h"

#include <csv/parser.hpp>

#include <csv/datasource/utf8/DataSource.hpp>
#include <csv/datasource/icu/DataSource.hpp>

@implementation DSFCSVParser

/// Convert the CSV Parser error to an Objc error
static ReturnState convertError(csv::State state)
{
	switch(state)
	{
		case csv::State::Complete:
			return ReturnState::Complete;
		case csv::State::Cancelled:
			return ReturnState::Cancelled;
		default:
			return ReturnState::Error;
	}
}

+ (ReturnState)parseUTF8String:(NSString* _Nonnull)str
					 separator:(char)separator
				 fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
				recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	csv::utf8::StringDataSource parser;
	parser.separator = separator;

	std::string temp = [str cStringUsingEncoding:NSUTF8StringEncoding];
	if (temp.empty())
	{
		// Text wasn't UTF8 encoded
		return ReturnState::Error;
	}
	parser.set(temp);

	return [DSFCSVParser parseUsingParser:parser
							fieldCallback:fieldCallback
						   recordCallback:recordCallback];
}

+ (ReturnState)parseUTF8File:(NSURL* _Nonnull)fileURL
				   separator:(char)separator
			   fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
			  recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	csv::utf8::FileDataSource parser;
	parser.separator = separator;

	if (!parser.open([fileURL fileSystemRepresentation]))
	{
		return Error;
	}

	return [DSFCSVParser parseUsingParser:parser
							fieldCallback:fieldCallback
						   recordCallback:recordCallback];
}

+ (ReturnState)parseUsingParser:(csv::IDataSource&)parser
				  fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
				 recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	// Callback when we receive a field
	csv::FieldCallback field = [&](const csv::field& field) -> bool
	{
		assert(fieldCallback != nil);
		return fieldCallback(field.row, field.column, [NSString stringWithUTF8String:field.content.c_str()]) == YES;
	};

	// Callback when we receive a record
	csv::RecordCallback record = [&](const csv::record& record) -> bool
	{
		assert(recordCallback != nil);
		NSMutableArray<NSString*>* rowData = [NSMutableArray arrayWithCapacity:record.content.size()];
		for (const auto& field : record.content)
		{
			[rowData addObject:[NSString stringWithUTF8String:field.content.c_str()]];
		}
		NSArray<NSString*>* result = [rowData copy];
		return recordCallback(record.row, result) == YES;
	};

	auto state = csv::parse(parser,
							(fieldCallback == nil) ? NULL : field,
							(recordCallback == nil) ? NULL : record);
	return convertError(state);
}

+ (ReturnState)parseData:(NSData* _Nonnull)rawData
			   separator:(char)separator
		   fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
		  recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	NSString* rawString;
	BOOL isLossy;

	[NSString stringEncodingForData:rawData
					encodingOptions:nil
					convertedString:&rawString
				usedLossyConversion:&isLossy];

	return [DSFCSVParser parseUTF8String:rawString
							   separator:separator
						   fieldCallback:fieldCallback
						  recordCallback:recordCallback];
}

#ifdef ALLOW_ICU_EXTENSIONS

+ (ReturnState)parseFile:(NSURL* _Nonnull)fileURL
			 icuCodepage:(const char* _Nullable)codepage
			   separator:(char)separator
		   fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
		  recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	csv::icu::FileDataSource parser;
	parser.separator = separator;

	if (parser.open([fileURL fileSystemRepresentation], codepage))
	{
		return [DSFCSVParser parseUsingParser:parser
								fieldCallback:fieldCallback
							   recordCallback:recordCallback];
	}
	return Error;
}

#endif

@end

#endif
