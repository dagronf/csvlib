
#include <iostream>
#include <algorithm>
#include <csv/parser.hpp>
#include <csv/datasource/icu/DataSource.hpp>

#include "command_line.hpp"

using namespace std;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

namespace {
	void PrintProgress (double percentage, size_t rowCount) {
		int val = (int) (percentage * 100);
		int lpad = (int) (percentage * PBWIDTH);
		int rpad = PBWIDTH - lpad;
		fprintf (stderr, "\r%3d%% [%.*s%*s] %ld", val, lpad, PBSTR, rpad, "", rowCount);
		fflush (stderr);
	}

	void ReplaceStringInPlace(std::string& subject,
							  const std::string& search,
							  const std::string& replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}
};

int main(int argc, const char * argv[]) {

	// Wrap everything in a try block.  Do this every time,
	// because exceptions will be thrown for problems.
	Arguments args;
	if (handle_command_args(argc, argv, args) == false) {
		return -1;
	}

	csv::icu::FileDataSource input;
	if (!input.open(args.inputFile.c_str(), args.codepage.length() > 0 ? args.codepage.c_str() : NULL)) {
		cerr << "Unable to open file" << endl;
		exit(-1);
	}

	if (args.type == "tsv") {
		input.separator = '\t';
	}

	if (args.separator != ',') {
		input.separator = args.separator;
	}

	int pp = -1;
	int total = 0;

	bool verbose = args.verbose;
	size_t limit = args.limit;

	auto recordAdder = [&pp, verbose, limit, &total](const csv::record& record, double complete) -> bool {

		if (total > 0) {
			cout << "\n";
		}

		total += 1;
		if (verbose && (int)(complete*100) != pp) {
			pp = (int)(complete*100);
			PrintProgress(complete, record.row);
		}

		bool lineStarted = false;
		for (const auto& field : record.content) {
			if (lineStarted) {
				lineStarted = true;
				cout << "\t";
			}
			std::string fieldValue = field.content;
			ReplaceStringInPlace(fieldValue, "\"", "\"\"");
			cout << "\"" << fieldValue << "\"\t";
		}

		if (limit > 0 && record.row == limit - 1) {
			PrintProgress(1.0, record.row + 1);
			return false;
		}

		// Only flush every 1000 records
		if ((total % 10000) == 0) {
			cout << flush;
		}

		return true;
	};
	csv::parse(input, NULL, recordAdder);

	if (args.verbose) {
		PrintProgress(1, total);
		cerr << endl;
	}

	return 0;
}
