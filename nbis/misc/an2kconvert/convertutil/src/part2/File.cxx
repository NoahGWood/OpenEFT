#include "part2/File.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <iostream>
#include <fstream>

namespace convert {
	namespace part2 {
		using namespace std;
		using namespace xercesc;

		File::File(string const& fileName, ValidationLevel vl, bool ebts) {
			auto_ptr<XMLElement> rootElement = readFile(fileName);
			infoRecord = auto_ptr<InformationRecord>(new InformationRecord(rootElement->removeChild(0), vl, ebts));
			list<FileContents> fileContents = infoRecord->getFileContents();
			for(list<FileContents>::const_iterator it = fileContents.begin(); it != fileContents.end(); it++) {
				FileContents const& fc = *it;
				records.push_back(new Record(fc.recordType, rootElement->removeChild(0), vl, ebts));
			}
		}

		File::File(part1::File const& part1File, bool ebts) {
			infoRecord = auto_ptr<InformationRecord>(new InformationRecord(part1File.getInfoRecord(), ebts));
			list<part1::Record*> const& part1Records = part1File.getRecords();
			for(list<part1::Record*>::const_iterator it = part1Records.begin(); it != part1Records.end(); it++) {
				part1::Record const& part1Record = *(*it);
				records.push_back(new Record(part1Record));
			}
		}

		InformationRecord const& File::getInfoRecord() const {
			return *infoRecord;
		}

		list<Record*> const& File::getRecords() const {
			return records;
		}

		auto_ptr<XMLElement> File::readFile(string const& fileName) const {
			try {
				XMLPlatformUtils::Initialize();
			} catch(XMLException const& e) {
				throw logic_error("Part2File.readFile(): Could not initialize Xerces");
			}

			auto_ptr<XercesDOMParser> parser(new XercesDOMParser());
			parser->setCreateCommentNodes(false);
			parser->setValidationScheme(XercesDOMParser::Val_Always);
			parser->setDoNamespaces(true);

			auto_ptr<ErrorHandler> errHandler((ErrorHandler*) new HandlerBase());
			parser->setErrorHandler(errHandler.get());

			try {
				parser->parse(fileName.c_str());
				auto_ptr<XMLElement> rootElement(new XMLElement(*(parser->getDocument()->getDocumentElement())));
				return rootElement;

			} catch(XMLException const& e) {
				throw ParseError("Could not parse XML input file");
			} catch(DOMException const& e) {
				throw ParseError("Could not parse XML input file");
			} catch(...) {
				throw ParseError("Could not parse XML input file");
			}
		}

		void File::validate() const {

		}

		void File::writeFile(string const& fileName) const {
			ofstream out(fileName.c_str(), ifstream::out|ifstream::binary|ifstream::trunc);
			if(out.fail()) {
				throw IOError("Could not open output file");
				//file failed to open properly
			}
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
			out << "<itl:NISTBiometricInformationExchangePackage xmlns:s=\"http://niem.gov/niem/structures/2.0\" xmlns:ansi-nist=\"http://niem.gov/niem/ansi-nist/2.0\" xmlns:nc=\"http://niem.gov/niem/niem-core/2.0\" xmlns:itl=\"http://biometrics.nist.gov/standard/2-2008\" xmlns:ebts=\"http://cjis.fbi.gov/fbi_ebts/beta_1.0.5\" xmlns:j=\"http://niem.gov/niem/domains/jxdm/4.0\">" << endl;
			out << "<!-- This file was created by an2kconvert Beta 5 -->" << endl;
			printTree(infoRecord->getRootElement(), 1, out);
			for(list<Record*>::const_iterator it = records.begin(); it != records.end(); it++) {
				Record const& part2Record = *(*it);
				printTree(part2Record.getRootElement(), 1, out);
			}
			out << "</itl:NISTBiometricInformationExchangePackage>" << endl;
			out.close();
		}

		void File::printTree(XMLElement const& elem, int depth, ostream& ostr) {
			for(int i = 0; i < depth; i++) {
				ostr << "\t";
			}
			ostr << elem.getElementId().openElement();
			if(elem.getText() != "") {
				ostr << elem.getText();
			} else {
				ostr << endl;
				for(list<XMLElement*>::const_iterator it = elem.getChildren().begin(); it != elem.getChildren().end(); it++) {
					printTree(*(*it), depth + 1, ostr);
				}
				for(int i = 0; i < depth; i++) {
					ostr << "\t";
				}
			}
			ostr << elem.getElementId().closeElement() << endl;
		}

		File::~File() {
			deleteContents<Record*>(records);
		}
	}
}
