//
//  command_line.cpp
//  csv_command_line
//
//  Created by Darren Ford on 22/5/19.
//  Copyright © 2019 Darren Ford. All rights reserved.
//

#include <tclap/CmdLine.h>

#include "command_line.hpp"

bool handle_command_args(int argc, const char * const * argv, Arguments& args) {
	try {
		TCLAP::CmdLine cmd("Convert a TSV or CSV file to a 'clean' UTF-8 encoded TSV output", ' ', "0.1");
		
		std::vector<std::string> allowed { "csv", "tsv" };
		TCLAP::ValuesConstraint<std::string> allowedVals( allowed );
		TCLAP::ValueArg<std::string> typeArg("t", "type", "The type of input file", false, "csv", &allowedVals);
		cmd.add( typeArg );

		TCLAP::ValueArg<char> separatorArg("s", "separator", "Separator character to use when decoding", false, ',', "separator character");
		cmd.add( separatorArg );

		TCLAP::ValueArg<std::string> codepageArg("c", "codepage", "The codepage to decode the input file if automatic detection doesn't work", false, "", "codepage");
		cmd.add( codepageArg );

		TCLAP::SwitchArg verboseArg("v", "verbose", "Display a progress bar during parsing");
		cmd.add( verboseArg );
		TCLAP::ValueArg<size_t> limitArg("l", "limit", "limit to the first <limit> records", false, 0, "limit");
		cmd.add( limitArg );
		TCLAP::UnlabeledValueArg<std::string> fileArg("file", "input file", true, "filenameString", "value");
		cmd.add( fileArg );
		
		// Parse the argv array.
		cmd.parse( argc, argv );
		
		args.type = typeArg.getValue();
		args.verbose = verboseArg.getValue();
		args.inputFile = fileArg.getValue();
		args.limit = limitArg.getValue();
		args.codepage = codepageArg.getValue();
		args.separator = separatorArg.getValue();
	}
	catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return false;
	}
	return true;
}
