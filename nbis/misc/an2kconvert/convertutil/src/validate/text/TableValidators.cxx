#include "validate/text/TableValidators.hxx"
#include "validate/ValidationMessage.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <iostream>

namespace convert {
	using namespace std;

	Table1Validator::Table1Validator()
	: MultipleValueValidator("NONE|WSQ20|JPEGB|JPEGL|JP2|JP2L|PNG") {}

	string Table1Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 1";
	}

	Table3Validator::Table3Validator()
	: MultipleValueValidator("UNK|GRAY|RGB|SRGB|YCC|SYCC") {}

	string Table3Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 3";
	}

	Table11Validator::Table11Validator()
	: MultipleNumericValidator("0|1|2|3|4|5|6|7|8|10|11|12|13|14|15|20|21|22|23|24|25|26|27|28|29") {}

	string Table11Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 11";
	}

	Table12Validator::Table12Validator()
	: MultipleNumericValidator("0|1|2|3|4|5|6|7|8|10|11|12|13|14|15|19") {}

	string Table12Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 12";
	}

	Table15Validator::Table15Validator()
	: MultipleValueValidator("A|B|C|D") {}

	string Table15Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 15";
	}

	Table18Validator::Table18Validator()
	: MultipleNumericValidator("0|1|10|11|12|13|14|15|20|30|40|50|51") {}

	string Table18Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 18";
	}

	Table19Validator::Table19Validator()
	: MultipleValueValidator("F|R|L|A|D") {}

	string Table19Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 19";
	}

	Table20Validator::Table20Validator()
	: MultipleValueValidator("GLASSES|HAT|SCARF|PHYSICAL|OTHER") {}

	string Table20Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 20";
	}

	Table21Validator::Table21Validator()
	: MultipleValueValidator("UNSPECIFIED|UNKNOWN PHOTO|DIGITAL CAMERA|SCANNER|UNKNOWN VIDEO|ANALOGUE VIDEO|DIGITAL VIDEO|VENDOR") {}

	string Table21Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 21";
	}

	Table23Validator::Table23Validator()
	: MultipleValueValidator("BLK|BLU|BRO|GRY|GRN|HAZ|MAR|MUL|PNK|XXX") {}

	string Table23Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 23";
	}

	Table24Validator::Table24Validator()
	: MultipleValueValidator("XXX|BAL|BLK|BLN|BRO|GRY|RED|SDY|WHI|BLU|GRN|ONG|PNK|PLE") {}

	string Table24Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 24";
	}

	Table27Validator::Table27Validator()
	: MultipleValueValidator("CONTROLLED|ASSISTED|OBSERVED|UNATTENDED|UNKNOWN") {}

	string Table27Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 27";
	}

	Table28Validator::Table28Validator()
	: MultipleValueValidator("HUMAN|ANIMAL|PLANT|FLAG|OBJECT|ABSTRACT|SYMBOL|OTHER") {}

	string Table28Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 28";
	}

	Table29Validator::Table29Validator()
	: MultipleValueValidator("MFACE|FFACE|ABFACE|MBODY|FBODY|ABBODY|ROLES|SPORT|MBPART|FBPART|ABPART|SKULL|MHUMAN|CAT|DOG|DOMESTIC|VICIOUS|HORSE|WILD|SNAKE|DRAGON|BIRD|INSECT|ABSTRACT|PARTS|MANIMAL|NARCOTICS|REDFL|YELFL|DRAW|ROSE|TULIP|LILY|MPLANT|FIRE|WEAP|PLANE|VESSEL|TRAIN|VEHICLE|MYTH|SPORT|NATURE|MOBJECTS|USA|STATE|NAZI|CONFED|BRIT|MFLAG|FIGURE|SLEEVE|BRACE|ANKLET|NECKLC|SHIRT|BODBND|HEDBND|MABSTRACT|NATION|POLITIC|MILITARY|FRATERNAL|PROFESS|GANG|MSYMBOLS|WORDING|FREEFRM|MISC") {}

	string Table29Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 29";
	}

	Table30Validator::Table30Validator()
	: MultipleValueValidator("BLACK|BROWN|GRAY|BLUE|GREEN|ORANGE|PURPLE|RED|YELLOW|WHITE|MULTI|OUTLINE") {}

	string Table30Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 30";
	}

	Table32Validator::Table32Validator()
	: MultipleValueValidator("EJI|TIP|FV1|FV2|FV3|FV4|PRX|DST|MED") {}

	string Table32Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 32";
	}

	Table35Validator::Table35Validator()
	: MultipleNumericValidator("20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36") {}

	string Table35Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 35";
	}

	Table39Validator::Table39Validator()
	: MultipleValueValidator("00000000|00000001|00000002|00000004|00000008|00000010|00000020|00000040|00000080|00000100|00000200|00000400|00000800|00001000|00002000|00004000|00008000|00010000|00020000|00040000|00080000") {}

	string Table39Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 39";
	}

	Table201Validator::Table201Validator()
	: MultipleValueValidator("NONE|WSQ20|JPEGB|JPEGL|JP2|JP2L|PNG") {}

	string Table201Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 201";
	}

	Table203Validator::Table203Validator()
	: MultipleValueValidator("UNK|GRAY|RGB|SRGB|YCC|SYCC") {}

	string Table203Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 203";
	}

	Table211Validator::Table211Validator()
	: MultipleNumericValidator("0|1|2|3|4|5|6|7|8|10|11|12|13|14|15|20|21|22|23|24|25|26|27|28|29") {}

	string Table211Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 211";
	}

	Table212Validator::Table212Validator()
	: MultipleNumericValidator("0|1|2|3|4|5|6|7|8|10|11|12|13|14|15|19") {}

	string Table212Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 212";
	}

	Table215Validator::Table215Validator()
	: MultipleValueValidator("A|B|C|D") {}

	string Table215Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 215";
	}

	Table219Validator::Table219Validator()
	: MultipleValueValidator("CONTROLLED|ASSISTED|OBSERVED|UNATTENDED|UNKNOWN") {}

	string Table219Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 219";
	}

	Table220Validator::Table220Validator()
	: MultipleNumericValidator("0|1|10|11|12|13|14|15|20|30|40|50|51") {}

	string Table220Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 220";
	}

	Table221Validator::Table221Validator()
	: MultipleValueValidator("GLASSES|HAT|SCARF|PHYSICAL|OTHER") {}

	string Table221Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 221";
	}

	Table223Validator::Table223Validator()
	: MultipleValueValidator("BLK|BLU|BRO|GRY|GRN|HAZ|MAR|MUL|PNK|XXX") {}

	string Table223Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 223";
	}

	Table226Validator::Table226Validator()
	: MultipleValueValidator("XXX|BAL|BLK|BLN|BRO|GRY|RED|SDY|WHI|BLU|GRN|ONG|PNK|PLE") {}

	string Table226Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 226";
	}

	Table227Validator::Table227Validator()
	: MultipleValueValidator("F|R|L|A|D") {}

	string Table227Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 227";
	}

	Table228Validator::Table228Validator()
	: MultipleValueValidator("UNSPECIFIED|UNKNOWN PHOTO|DIGITAL CAMERA|SCANNER|UNKNOWN VIDEO|ANALOGUE VIDEO|DIGITAL VIDEO|VENDOR") {}

	string Table228Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 228";
	}

	Table229Validator::Table229Validator()
	: MultipleValueValidator("BLACK|BROWN|GRAY|BLUE|GREEN|ORANGE|PURPLE|RED|YELLOW|WHITE|MULTI|OUTLINE") {}

	string Table229Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 229";
	}

	Table230Validator::Table230Validator()
	: MultipleValueValidator("HUMAN|ANIMAL|PLANT|FLAG|OBJECT|ABSTRACT|SYMBOL|OTHER") {}

	string Table230Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 230";
	}

	Table231Validator::Table231Validator()
	: MultipleValueValidator("MFACE|FFACE|ABFACE|MBODY|FBODY|ABBODY|ROLES|SPORT|MBPART|FBPART|ABPART|SKULL|MHUMAN|CAT|DOG|DOMESTIC|VICIOUS|HORSE|WILD|SNAKE|DRAGON|BIRD|INSECT|ABSTRACT|PARTS|MANIMAL|NARCOTICS|REDFL|YELFL|DRAW|ROSE|TULIP|LILY|MPLANT|FIRE|WEAP|PLANE|VESSEL|TRAIN|VEHICLE|MYTH|SPORT|NATURE|MOBJECTS|USA|STATE|NAZI|CONFED|BRIT|MFLAG|FIGURE|SLEEVE|BRACE|ANKLET|NECKLC|SHIRT|BODBND|HEDBND|MABSTRACT|NATION|POLITIC|MILITARY|FRATERNAL|PROFESS|GANG|MSYMBOLS|WORDING|FREEFRM|MISC") {}

	string Table231Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 231";
	}

	Table233Validator::Table233Validator()
	: MultipleValueValidator("EJI|TIP|FV1|FV2|FV3|FV4|PRX|DST|MED") {}

	string Table233Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 233";
	}

	Table235Validator::Table235Validator()
	: MultipleNumericValidator("20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36") {}

	string Table235Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 235";
	}

	Table240Validator::Table240Validator()
	: MultipleValueValidator("00000000|00000001|00000002|00000004|00000008|00000010|00000020|00000040|00000080|00000100|00000200|00000400|00000800|00001000|00002000|00004000|00008000|00010000|00020000|00040000|00080000") {}

	string Table240Validator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 240";
	}
}
