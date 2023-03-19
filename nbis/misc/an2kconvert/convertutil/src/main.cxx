#include "Errors.hxx"
#include "ITLPackage.hxx"
#include "validate/Validation.hxx"

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using namespace convert;
using namespace std;

FileType guessFileType(string const& fileName, bool ebts) {
	ifstream in(fileName.c_str(), ifstream::in|ifstream::binary);
	if(in.fail()) {
		throw IOError("Could not open input file");
		//file failed to open properly, file may not exist
	}
	string bytes;
	for(int i = 0; i < 10; i++) {
		if(!in.fail()) {
			bytes += in.get();
		}
	}
	in.close();

	if(bytes[0] == '1') {
		return ebts ? EBTS_PART1 : AN_PART1;
	} else {
		return ebts ? EBTS_PART2 : AN_PART2;
	}
}

void printHelp() {
	cout << "Usage" << endl;
	cout << "\tan2kconvert -h" << endl;
	cout << "\tan2kconvert -c [-o] [-s|-r] [--ebts] INPUTFILE OUTPUTFILE" << endl;
	cout << "\tan2kconvert -v [-s|-r] [--ebts] INPUTFILE" << endl;
	cout << "Options" << endl;
	cout << "\t-h, --help" << endl;
	cout << "\t\tprints help message" << endl;
	cout << "\t-v, --validate" << endl;
	cout << "\t\tvalidates the INPUTFILE" << endl;
	cout << "\t-c, --convert" << endl;
	cout << "\t\tconverts the INPUTFILE and outputs it to OUTPUTFILE" << endl;
	cout << "\t\tIf INPUTFILE is a Part 1 file, it will be converted to a Part 2 file and written to OUTPUTFILE." << endl;
	cout << "\t\tIf INPUTFILE is a Part 2 file, it will be converted to a Part 1 file and written to OUTPUTFILE." << endl;
	cout << "\t-o, --overwrite" << endl;
	cout << "\t\toverwrites the OUTPUTFILE if it exists, by default the OUTPUTFILE will not be overwritten" << endl;
	cout << "\t-s, --strict" << endl;
	cout << "\t\tenables strict validation of the INPUTFILE, relaxed validation is enabled by default" << endl;
	cout << "\t-r, --relaxed" << endl;
	cout << "\t\tenables relaxed validation of the INPUTFILE, relaxed validation is enabled by default" << endl;
	cout << "\t--ebts" << endl;
	cout << "\t\tenables support for records with EBTS data" << endl;
}

void printError() {
	cout << "Error: invalid command syntax" << endl;
	printHelp();
}

int main(int argc, char* argv[]) {
	bool help = false;
	bool validate = false;
	bool convert = false;
	bool overwrite = false;
	bool strict = false;
	bool relaxed = false;
	bool fileExists = false;
	bool ebts = false;
	ValidationLevel vl;
	FileType fileType;
	string inputFile;
	string outputFile;

	//Process command line flags
	int i = 1;
	while(i < argc && argv[i][0] == '-') {
		if(string(argv[i]) == "--help") {
			help = true;
		} else if(string(argv[i]) == "--validate") {
			validate = true;
		} else if(string(argv[i]) == "--convert") {
			convert = true;
		} else if(string(argv[i]) == "--overwrite") {
			overwrite = true;
		} else if(string(argv[i]) == "--strict") {
			strict = true;
		} else if(string(argv[i]) == "--relaxed") {
			relaxed = true;
		} else if(string(argv[i]) == "--ebts") {
			ebts = true;
		} else {
			for(char* c = argv[i] + 1; *c != '\0'; c++) {
				if(*c == 'h') {
					help = true;
				} else if(*c == 'v') {
					validate = true;
				} else if(*c == 'c') {
					convert = true;
				} else if(*c == 'o') {
					overwrite = true;
				} else if(*c == 's') {
					strict = true;
				} else if(*c == 'r') {
					relaxed = true;
				} else {
					printError();
					return 1;
				}
			}
		}
		i++;
	}

	//check validity of command line args
	if(help) {
		if(validate || convert || overwrite || strict || relaxed || ebts) {
			printError();
			return 1;
		} else {
			printHelp();
		}
	} else if(validate) {
		if(convert || overwrite || (strict && relaxed) || argc - i != 1) {
			printError();
			return 1;
		} else {
			if(strict) {
				vl = Strict;
			} else {
				vl = Relaxed;
			}
			inputFile = string(argv[argc - 1]);
			try {
				fileType = guessFileType(inputFile, ebts);
				ITLPackage package(inputFile, fileType, vl);
			} catch(IOError& e) {
				cout << "I/O Error:" << endl;
				cout << e.what() << endl;
			}  catch(ParseError& e) {
				cout << "Parse Error:" << endl;
				cout << e.what() << endl;
			} catch(ValidationError& e) {
				cout << e.what() << endl;
			} catch(logic_error& e) {
				cout << "Program Logic Error:" << endl;
				cout << e.what() << endl;
			}
		}
	} else if(convert) {
		if((strict && relaxed) || argc - i != 2) {
			printError();
			return 1;
		} else {
			if(strict) {
				vl = Strict;
			} else {
				vl = Relaxed;
			}
			inputFile = string(argv[argc - 2]);
			outputFile = string(argv[argc - 1]);

			if(!overwrite) {
				struct stat stFileInfo;
				if(!stat(outputFile.c_str(), &stFileInfo)) {
					fileExists = true;
				}
			}
			if(overwrite || !fileExists) {
				try {
					fileType = guessFileType(inputFile, ebts);
					ITLPackage package(inputFile, fileType, vl);
					if(isPart1(fileType)) {
						package.outputPart2File(outputFile);
					}
					if(isPart2(fileType)) {
						package.outputPart1File(outputFile);
					}
				} catch(IOError& e) {
					cout << "I/O Error:" << endl;
					cout << e.what() << endl;
				}  catch(ParseError& e) {
					cout << "Parse Error:" << endl;
					cout << e.what() << endl;
				} catch(ValidationError& e) {
					cout << e.what() << endl;
				} catch(logic_error& e) {
					cout << "Program Logic Error:" << endl;
					cout << e.what() << endl;
				}
			}
		}
	} else {
		printError();
		return 1;
	}

	return 0;
}
