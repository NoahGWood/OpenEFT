#ifndef RECORDCONVERTER_HXX
#define RECORDCONVERTER_HXX

#include "RecordType.hxx"
#include "convert/Converter.hxx"
#include "part1/Record.hxx"
#include "part2/Record.hxx"

#include <list>
#include <memory>
#include <stack>
#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	namespace part1 {
		class Record;
	}

	namespace part2 {
		class Record;
	}

	class RecordConverter {
	public:
		~RecordConverter();
		static void convertRecord(part1::Record const& part1Record, part2::Record& part2Record);
		static void convertRecord(part2::Record const& part2Record, part1::Record& part1Record);
		void convert(part1::Record const& part1Record, part2::Record& part2Record);
		void convert(part2::Record const& part2Record, part1::Record& part1Record);

	private:
		void postProcess(part1::Record& part1Record);
		explicit RecordConverter(RecordVariant recordVariant);
		void addNode(Converter* converter);
		void finishNode(string const& elementName, XMLNamespace ns);
		void addLeafNode(Converter* converter);
		void configureType1AConverter();
		void configureType1BConverter();
		void configureType2AConverter();
		void configureType2BConverter();
		void configureType3Converter();
		void configureType4Converter();
		void configureType5Converter();
		void configureType6Converter();
		void configureType3_6Converter(RecordType recordType);
		void configureType7Converter();
		void configureType8AConverter();
		void configureType8BConverter();
		void configureType9AConverter();
		void configureType9BConverter();
		void configureType9CConverter();
		void configureType9DConverter();
		void configureType10AConverter();
		void configureType10BConverter();
		void configureType13AConverter();
		void configureType13BConverter();
		void configureType13CConverter();
		void configureType14AConverter();
		void configureType14BConverter();
		void configureType15Converter();
		void configureType16Converter();
		void configureType17Converter();
		void configureType99Converter();

		auto_ptr<Converter> rootNode;
		stack<Converter*, list<Converter*> > nodeStack;
	};
}

#endif
