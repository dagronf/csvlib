//
//  command_line.hpp
//  csv_command_line
//
//  Created by Darren Ford on 22/5/19.
//  Copyright © 2019 Darren Ford. All rights reserved.
//

#pragma once

#include <string>

struct Arguments {
	std::string type;
	char separator;
	bool verbose;
	std::string inputFile;
	std::string codepage;
	size_t limit;
};

bool handle_command_args(int argc, const char * const * argv, Arguments& args);
