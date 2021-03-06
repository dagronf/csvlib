//
//  DSFCSVParser.h
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

#import <Foundation/Foundation.h>

@interface DSFCSVDataSource: NSObject

+ (DSFCSVDataSource* _Nullable)dataSourceWithUTF8String:(NSString* _Nonnull)str
											  separator:(char)separator;

+ (DSFCSVDataSource* _Nullable)dataSourceWithUTF8FileURL:(NSURL* _Nonnull)fileURL
											   separator:(char)separator;

+ (DSFCSVDataSource* _Nullable)dataSourceWithData:(NSData* _Nonnull)rawData
										separator:(char)separator;

#ifdef ALLOW_ICU_EXTENSIONS

+ (DSFCSVDataSource* _Nullable)dataSourceWithFileURL:(NSURL* _Nonnull)fileURL
										 icuCodepage:(const char* _Nullable)codepage
										   separator:(char)separator;

#endif

@end


@interface DSFCSVParser : NSObject

typedef NS_ENUM(NSUInteger, ReturnState) {
	/// Content has been completed
	Complete = 0,
	/// Caller cancelled
	Cancelled = 1,
	/// Error
	Error = 2
};

typedef BOOL (^CSVFieldCallback)(const NSUInteger /* row */,
								 const NSUInteger /* field */,
								 const NSString* _Nonnull /* field content */);

typedef BOOL (^CSVRecordCallback)(const NSUInteger /* row */,
								  const NSArray<NSString*>* _Nonnull /* row content */,
								  const CGFloat);		// progress complete (0.0 -> 1.0)

+ (ReturnState)parseWithDataSource:(DSFCSVDataSource* _Nonnull)dataSource
					 fieldCallback:(CSVFieldCallback _Nullable)fieldCallback
					recordCallback:(CSVRecordCallback _Nullable)recordCallback;

@end

#endif
