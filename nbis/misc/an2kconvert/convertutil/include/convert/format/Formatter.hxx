#ifndef FORMATTER_HXX
#define FORMATTER_HXX

#include <string>

namespace convert {
	using namespace std;

	class Formatter {
	public:
		virtual string formatPart1(string const& part2Value) const;
		virtual string formatPart2(string const& part1Value) const;
	};

	class Base64Formatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class DateFormatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class IDCFormatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class ItemSplitFormatter : public Formatter {
	public:
		ItemSplitFormatter(char delimiter, size_t index);
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;

	private:
		char delimiter;
		size_t index;
	};

	class RidgeCountIndicatorFormatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class StandardIndicatorFormatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class UTCDateFormatter : public Formatter {
	public:
		string formatPart1(string const& part2Value) const;
		string formatPart2(string const& part1Value) const;
	};

	class Type8LLFormatter : public Formatter {
	public:
		string formatPart2(string const& part1Value) const;
	};
}

#endif
