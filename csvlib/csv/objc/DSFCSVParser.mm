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

@interface DSFCSVDataSource ()
{
	std::unique_ptr<csv::IDataSource> __source;
}

- (csv::IDataSource&)source;

@end

@implementation DSFCSVDataSource

- (csv::IDataSource&)source
{
	return *__source;
}


- (void)dealloc
{
	/// OBJC++: C++ members do not automatically have their destructors called on dealloc
	///         thus this will leak unless we explicitly clean up.
	__source.reset();
}

+ (DSFCSVDataSource* _Nullable)dataSourceWithUTF8String:(NSString*)str
									separator:(char)separator
{
	std::string temp = [str cStringUsingEncoding:NSUTF8StringEncoding];
	if (temp.empty())
	{
		// Text wasn't UTF8 encoded
		return nil;
	}

	auto data = std::make_unique<csv::utf8::StringDataSource>();
	data->set(temp);
	data->separator = separator;

	DSFCSVDataSource* dataSource = [[DSFCSVDataSource alloc] init];
	dataSource->__source = std::move(data);

	return dataSource;
}

+ (DSFCSVDataSource* _Nullable)dataSourceWithUTF8FileURL:(NSURL* _Nonnull)fileURL
									 separator:(char)separator
{
	auto data = std::make_unique<csv::utf8::FileDataSource>();
	if (!data->open([fileURL fileSystemRepresentation]))
	{
		return nil;
	}

	data->separator = separator;

	DSFCSVDataSource* dataSource = [[DSFCSVDataSource alloc] init];
	dataSource->__source = std::move(data);

	return dataSource;
}

+ (DSFCSVDataSource* _Nullable)dataSourceWithData:(NSData* _Nonnull)rawData
							  separator:(char)separator
{
	NSString* rawString;
	BOOL isLossy;

	[NSString stringEncodingForData:rawData
					encodingOptions:nil
					convertedString:&rawString
				usedLossyConversion:&isLossy];

	if (rawString != nil)
	{
		return [DSFCSVDataSource dataSourceWithUTF8String:rawString separator:separator];
	}

	return nil;
}

#ifdef ALLOW_ICU_EXTENSIONS

+ (DSFCSVDataSource* _Nullable)dataSourceWithFileURL:(NSURL* _Nonnull)fileURL
							   icuCodepage:(const char* _Nullable)codepage
								 separator:(char)separator
{
	auto data = std::make_unique<csv::icu::FileDataSource>();
	if (!data->open([fileURL fileSystemRepresentation], codepage))
	{
		return nil;
	}

	data->separator = separator;

	DSFCSVDataSource* dataSource = [[DSFCSVDataSource alloc] init];
	dataSource->__source = std::move(data);

	return dataSource;
}

#endif

@end



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

+ (ReturnState)parseWithDataSource:(DSFCSVDataSource*)dataSource
					 fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
					recordCallback:(CSVRecordCallback _Nullable)recordCallback
{
	csv::IDataSource& source = [dataSource source];

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

	auto state = csv::parse(source,
							(fieldCallback == nil) ? NULL : field,
							(recordCallback == nil) ? NULL : record);
	return convertError(state);
}

@end

#endif
