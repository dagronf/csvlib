# Simple CSV/TSV parser in C++

## Why another CSV parser?

Have you ever just wanted or needed a CSV or TSV parser that :

1. Doesn't need to have major dependencies on large libraries like Boost?
2. Written in C++, but can be bound to other languages like Objective-C and Swift with minimal dependencies?
3. Handles fields containing line breaks?
4. Supports different line endings like, \r, \n and \r\n?
4. Can handle CSV files containing mixtures of quoted and unquoted fields?
4. Handles files or strings in different encodings? (using ICU)
5. Can choose to ignore blank lines?
6. Simple callback interface on a per field and/or record basis?

Well, here's another one!  I got sick and tired of how many times I needed this functionality, went looking online for a good library then discovered that it didn't handle some aspect of the standard.  Or, it required Boost or some other large component that I didn't want to include in my codebase.

## Features

### Encoding

* UTF8 support with no additional libraries
* Other encoding types (eg. EUR-KR) by linking against ICU libraries.  Automatic and manual encoding detection support
* Ability to skip blank lines
* Support for comments

### Different line endings

Support for `\r`, `\n` and `\r\n`, even mixed line endings in the same file or string

### Embedded line breaks
```
Feelings, Thoughts
"angry
yet hopeful", none
```

### Embedded quotes

```
Name, Address
"John ""The Boss"" McMillan", No fixed address
Fred Irish 🇮🇪, No fixed address
```

Also allows for 'bad' quotes

```
cat, "dog", fi"sh
```

### Embedded Comments

Define a comment identifier (eg `%`) to ignore lines that start with that identifier
```
% Climate data
% Years 1870 to current
1870	1.123	4444
1871	1.122	3434		
```

## Tech stuff

It attempts to be as close to [RFC 4180](https://tools.ietf.org/html/rfc4180) as possible, with some wiggle room on certain aspects (eg. support for different line endings such as `\r`, `\n` along with `\r\n` defined in the standard).  This also provides (optional) commenting support, so if your CSV file contains comments (eg. `# Report results`) this library can skip them as needed.

Allows the caller to define a custom column separator, so to parse a tab-separated file instead you can set the character separator for parsing.

Column offset is returned for each field read, and row offset for each row, meaning that if you have structured data, you can (eg.) calculate a total for every field in column 2 of your dataset.

Uses C++ lambda callbacks to pass fields and/or entire records back to the calling process.  You only want the first 20 records in a file?  Return `false` to one of these lambda calls at any time and the parsing process completes.  Want to stop parsing when you find a particular text field in your input data?  You can do that too by returning `false` to the field callback.

By design, this library doesn't try to convert data to specific types as it is read.  Fields are purely UTF8 encoded strings when they are returned to the caller.  It is up to you and your calling code to do meaningful things with the returned data.

For larger files, the parser can take a long time to complete.  This library does not provide any support for threading, rather relies on the caller to perform threading (ie. call the parse methods on a background thread) as needed.

This library doesn't enforce columns, or 'expected' values. If the first row in your file has 10 columns and the second has only 8, then that's what you'll get.  There are no column  formatting rules, it is up to you to handle the data as it is returned.

This library is not optimized for speed (although it is pretty fast).  If you need a blindingly fast c++ csv parser I'd suggest looking [here](https://github.com/ben-strasser/fast-cpp-csv-parser).

## Support for ICU (International Components for Unicode)

See more about [ICU here](http://site.icu-project.org).

If you need to be able to read CSV or TSV files in C++ that (potentially) are encoded for a different language then this library also has you covered.  To get basic UTF-8 functionality you don't need ICU, however if you link against the ICU libraries you can use the full functionality of ICU to read other CSV file encodings.

Using ICU allows the parser to attempt to guess the encoding of a text file automatically, or you can pass the encoding as a parameter to the `parse` call.

## Examples

### C++ UTF-8

#### Read all records in a file, returning each field along with the complete records as the file is read 

```cpp
csv::datasource::utf8::FileDataSource input;

if (!input.open("<some-csv-file>.csv")) {
   assert(false);
}

csv::parse(input,
   [](const csv::field& field) -> bool {
      // Do something with 'field'
      return true;
   },
      [](const csv::record& record) -> bool {
      // Do something with 'record'
      return true;
   }
);

```

#### Read only the first 20 records in a file ignoring field callbacks
```cpp
csv::datasource::utf8::FileDataSource input;
if (!input.open("<some-csv-file>.csv")) {
   assert(false);
}

const size_t maxRows = 20;
csv::parse(input, NULL,
   [=](const csv::record& record) -> bool {
      return record.row < maxRows - 1;
   }
);
```

#### Use ICU to read CSV from a file with unknown encoding (EUC-KR)

(Requires linking against the appropriate ICU libraries)

```cpp
csv::datasource::icu::FileParser parser;
if (!parser.open("/tmp/korean.csv", NULL)) {
   assert(false);
}

csv::parse(parser,
   [](const csv::field& field) -> bool {
      std::cout << "* Field: (" << field.column << " : " << field.content << ")" << std::endl;
      return true;
   },
   [](const csv::record& record) -> bool {
      return true;
   }
);
```


### Objective-C Example

#### Read CSV row data from an NSString

```objc
[DSFCSVParser parseUTF8String:@"cat, dog, fish\rwhale, narwhal, swordfish"
                fieldCallback:^BOOL(const NSUInteger row, const NSUInteger column, const NSString* field) {
                   NSLog(@"Field:\n%@", field);
                   return YES;
                } 
                recordCallback:^BOOL(const NSUInteger row, const NSArray<NSString*>* record) {
                   NSLog(@"Record:\n%@", record);
                   return YES;
                }];
```

#### Read string data from an NSData object using CoreFoundation to infer the encoding

(CoreFoundation has the ability to convert from NSData to NSString by inferring the encoding without the need to link against ICU which is quite nice.)

```objc
NSURL* fileURL = <some file URL>
NSData* data = [NSData dataWithContentsOfURL:url];
[DSFCSVParser parseData:data
          fieldCallback:^BOOL(const NSUInteger row, const NSUInteger column, const NSString* field) {
              NSLog(@"Field:\n%@", field);
              return YES;
          }
          recordCallback:^BOOL(const NSUInteger row, const NSArray<NSString*>* record) {
              NSLog(@"Record:\n%@", record);
              return YES;
          }];
```
