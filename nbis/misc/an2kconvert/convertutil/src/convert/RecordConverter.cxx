#include "convert/RecordConverter.hxx"
#include "convert/CompositeField.hxx"
#include "convert/Converter.hxx"
#include "convert/ItemIncrementer.hxx"
#include "convert/LiteralConverter.hxx"
#include "convert/SubfieldIncrementer.hxx"
#include "convert/SimpleField.hxx"
#include "convert/SimpleItem.hxx"
#include "convert/SimpleListItem.hxx"
#include "convert/SimpleListSubfield.hxx"
#include "convert/format/Formatter.hxx"
#include "part2/ElementID.hxx"
#include "part2/XMLElement.hxx"

#include <iostream>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	RecordConverter::~RecordConverter() {
		deleteContents<Converter*, list<Converter*> >(nodeStack);
	}

	void RecordConverter::convertRecord(part1::Record const& part1Record, part2::Record& part2Record) {
		auto_ptr<RecordConverter> rc(new RecordConverter(part1Record.getRecordVariant()));
		rc->convert(part1Record, part2Record);
	}

	void RecordConverter::convertRecord(part2::Record const& part2Record, part1::Record& part1Record) {
		auto_ptr<RecordConverter> rc(new RecordConverter(part2Record.getRecordVariant()));
		rc->convert(part2Record, part1Record);
	}

	void RecordConverter::convert(part1::Record const& part1Record, part2::Record& part2Record) {
		if(!nodeStack.empty()) {
			throw logic_error("RecordConverter.convert(): didn't finish initializing converter");
		}
		if(rootNode.get() == NULL) {
			throw logic_error("RecordConverter.convert(): didn't initialize converter");
		}
		auto_ptr<list<XMLElement*> > elemList = rootNode->convert(part1Record);
		auto_ptr<XMLElement> elem(elemList->front());
		elemList->pop_front();
		part2Record.setRootElement(elem);
	}

	void RecordConverter::convert(part2::Record const& part2Record, part1::Record& part1Record) {
		if(!nodeStack.empty()) {
			throw logic_error("RecordConverter.convert(): didn't finish initializing converter");
		}
		if(rootNode.get() == NULL) {
			throw logic_error("RecordConverter.convert(): didn't initialize converter");
		}
		rootNode->convert(part2Record.getRootElement(), part1Record);
		postProcess(part1Record);
	}

	void RecordConverter::postProcess(part1::Record& part1Record) {
		if(part1Record.getRecordType() == TYPE3 || part1Record.getRecordType() == TYPE4 || part1Record.getRecordType() == TYPE5 || part1Record.getRecordType() == TYPE6) {
			FieldID fieldId = FieldID(part1Record.getRecordType(), 4);
			Field& field = part1Record.getField(fieldId);
			while(field.subfieldsCount() < 6) {
				field.addSubfield("255");
			}
		}
	}

	RecordConverter::RecordConverter(RecordVariant recordVariant) {
		switch(recordVariant) {
		case TYPE1_A: configureType1AConverter(); break;
		case TYPE1_B: configureType1BConverter(); break;
		case TYPE2_A: configureType2AConverter(); break;
		case TYPE2_B: configureType2BConverter(); break;
		case TYPE3_A: configureType3Converter(); break;
		case TYPE4_A: configureType4Converter(); break;
		case TYPE5_A: configureType5Converter(); break;
		case TYPE6_A: configureType6Converter(); break;
		case TYPE7_A: configureType7Converter(); break;
		case TYPE8_A: configureType8AConverter(); break;
		case TYPE8_B: configureType8BConverter(); break;
		case TYPE9_A: configureType9AConverter(); break;
		case TYPE9_B: configureType9BConverter(); break;
		case TYPE9_C: configureType9CConverter(); break;
		case TYPE9_D: configureType9DConverter(); break;
		case TYPE10_A: configureType10AConverter(); break;
		case TYPE10_B: configureType10BConverter(); break;
		case TYPE13_A: configureType13AConverter(); break;
		case TYPE13_B: configureType13BConverter(); break;
		case TYPE13_C: configureType13CConverter(); break;
		case TYPE14_A: configureType14AConverter(); break;
		case TYPE14_B: configureType14BConverter(); break;
		case TYPE15_A: configureType15Converter(); break;
		case TYPE16_A: configureType16Converter(); break;
		case TYPE17_A: configureType17Converter(); break;
		case TYPE99_A: configureType99Converter(); break;
		}
	}

	void RecordConverter::addNode(Converter* converter) {
		nodeStack.push(converter);
	}

	void RecordConverter::finishNode(string const& elementName, XMLNamespace ns) {
		if(nodeStack.empty()) {
			throw logic_error("RecordConverter.finishNode(): can't finish empty node");
		}
		if(ElementID(elementName, ns) != nodeStack.top()->getElementId()) {
			throw logic_error("RecordConverter.finishNode(): elementId doesn't match top node");
		}
		auto_ptr<Converter> node(nodeStack.top());
		nodeStack.pop();
		if(nodeStack.empty()) {
			rootNode = node;
		} else {
			nodeStack.top()->addChild(node);
		}
	}

	void RecordConverter::addLeafNode(Converter* converter) {
		if(nodeStack.empty()) {
			throw logic_error("RecordConverter.addLeafNode(): can't add leaf node to empty parent");
		}
		nodeStack.top()->addChild(auto_ptr<Converter>(converter));
	}

	void RecordConverter::configureType1AConverter() {
		addNode(new Converter("PackageInformationRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "01"));

			addNode(new Converter("Transaction", AN));
				addNode(new Converter("TransactionDate", AN));
					addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_1_DAT, new DateFormatter));
				finishNode("TransactionDate", AN);

				addNode(new Converter("TransactionDestinationOrganization", AN));
					addNode(new Converter("OrganizationIdentification", NC));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_DAI));
					finishNode("OrganizationIdentification", NC);
				finishNode("TransactionDestinationOrganization", AN);

				addNode(new Converter("TransactionOriginatingOrganization", AN));
					addNode(new Converter("OrganizationIdentification", NC));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_ORI));
					finishNode("OrganizationIdentification", NC);
				finishNode("TransactionOriginatingOrganization", AN);

				addNode(new Converter("TransactionUTCDate", AN));
					addLeafNode(new SimpleField("DateTime", NC, FieldID::TYPE_1_GMT, new UTCDateFormatter));
				finishNode("TransactionUTCDate", AN);

				addNode(new Converter("TransactionControlIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_TCN));
				finishNode("TransactionControlIdentification", AN);

				addNode(new Converter("TransactionControlReferenceIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_TCR));
				finishNode("TransactionControlReferenceIdentification", AN);

				addNode(new Converter("TransactionDomain", AN));
					addNode(new Converter("DomainVersionNumberIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_DOM, SubfieldIndex(0), ItemIndex(1)));
					finishNode("DomainVersionNumberIdentification", AN);
					addLeafNode(new SimpleItem("OrganizationName", AN, FieldID::TYPE_1_DOM, SubfieldIndex(0), ItemIndex(0)));
				finishNode("TransactionDomain", AN);

				addNode(new Converter("TransactionImageResolutionDetails", AN));
					addLeafNode(new SimpleField("NativeScanningResolutionValue", AN, FieldID::TYPE_1_NSR));
					addLeafNode(new SimpleField("NominalTransmittingResolutionValue", AN, FieldID::TYPE_1_NTR));
				finishNode("TransactionImageResolutionDetails", AN);

				addLeafNode(new CompositeField("TransactionMajorVersionValue", AN, FieldID::TYPE_1_VER, SubstrIndex(0), SubstrLen(2)));
				addLeafNode(new CompositeField("TransactionMinorVersionValue", AN, FieldID::TYPE_1_VER, SubstrIndex(2), SubstrLen(2)));
				addLeafNode(new SimpleField("TransactionPriorityValue", AN, FieldID::TYPE_1_PRY));
				addLeafNode(new SimpleField("TransactionCategoryCode", AN, FieldID::TYPE_1_TOT));

				addNode(new Converter("TransactionContentSummary", AN));
					addLeafNode(new SimpleItem("ContentFirstRecordCategoryCode", AN, FieldID::TYPE_1_CNT, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("ContentRecordCount", AN, FieldID::TYPE_1_CNT, SubfieldIndex(0), ItemIndex(1)));
					addNode(new SubfieldIncrementer("ContentRecordSummary", AN));
						addNode(new Converter("ImageReferenceIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_CNT, SubfieldIndex(1), ItemIndex(1), new IDCFormatter));
						finishNode("ImageReferenceIdentification", AN);
						addLeafNode(new SimpleItem("RecordCategoryCode", AN, FieldID::TYPE_1_CNT, SubfieldIndex(1), ItemIndex(0)));
					finishNode("ContentRecordSummary", AN);
				finishNode("TransactionContentSummary", AN);

				addNode(new SubfieldIncrementer("TransactionCharacterSetDirectory", AN));
					addLeafNode(new SimpleItem("CharacterSetCommonNameCode", AN, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("CharacterSetIndexCode", AN, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("CharacterSetVersionIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(2)));
					finishNode("CharacterSetVersionIdentification", AN);
				finishNode("TransactionCharacterSetDirectory", AN);

			finishNode("Transaction", AN);
		finishNode("PackageInformationRecord", ITL);
	}

	void RecordConverter::configureType1BConverter() {
		addNode(new Converter("PackageInformationRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "01"));

			addNode(new Converter("Transaction", EBTS));
				addNode(new Converter("TransactionDate", AN));
					addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_1_DAT, new DateFormatter));
				finishNode("TransactionDate", AN);

				addNode(new Converter("TransactionDestinationOrganization", AN));
					addNode(new Converter("OrganizationIdentification", NC));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_DAI));
					finishNode("OrganizationIdentification", NC);
				finishNode("TransactionDestinationOrganization", AN);

				addNode(new Converter("TransactionOriginatingOrganization", AN));
					addNode(new Converter("OrganizationIdentification", NC));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_ORI));
					finishNode("OrganizationIdentification", NC);
				finishNode("TransactionOriginatingOrganization", AN);

				addNode(new Converter("TransactionUTCDate", AN));
					addLeafNode(new SimpleField("DateTime", NC, FieldID::TYPE_1_GMT, new UTCDateFormatter));
				finishNode("TransactionUTCDate", AN);

				addNode(new Converter("TransactionControlIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_TCN));
				finishNode("TransactionControlIdentification", AN);

				addNode(new Converter("TransactionControlReferenceIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_1_TCR));
				finishNode("TransactionControlReferenceIdentification", AN);

				addNode(new Converter("TransactionDomain", AN));
					addNode(new Converter("DomainVersionNumberIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_DOM, SubfieldIndex(0), ItemIndex(1)));
					finishNode("DomainVersionNumberIdentification", AN);
					addLeafNode(new SimpleItem("OrganizationName", AN, FieldID::TYPE_1_DOM, SubfieldIndex(0), ItemIndex(0)));
				finishNode("TransactionDomain", AN);

				addNode(new Converter("TransactionImageResolutionDetails", AN));
					addLeafNode(new SimpleField("NativeScanningResolutionValue", AN, FieldID::TYPE_1_NSR));
					addLeafNode(new SimpleField("NominalTransmittingResolutionValue", AN, FieldID::TYPE_1_NTR));
				finishNode("TransactionImageResolutionDetails", AN);

				addLeafNode(new CompositeField("TransactionMajorVersionValue", AN, FieldID::TYPE_1_VER, SubstrIndex(0), SubstrLen(2)));
				addLeafNode(new CompositeField("TransactionMinorVersionValue", AN, FieldID::TYPE_1_VER, SubstrIndex(2), SubstrLen(2)));
				addLeafNode(new SimpleField("TransactionPriorityValue", AN, FieldID::TYPE_1_PRY));

				addNode(new Converter("TransactionContentSummary", AN));
					addLeafNode(new SimpleItem("ContentFirstRecordCategoryCode", AN, FieldID::TYPE_1_CNT, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("ContentRecordCount", AN, FieldID::TYPE_1_CNT, SubfieldIndex(0), ItemIndex(1)));
					addNode(new SubfieldIncrementer("ContentRecordSummary", AN));
						addNode(new Converter("ImageReferenceIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_CNT, SubfieldIndex(1), ItemIndex(1), new IDCFormatter));
						finishNode("ImageReferenceIdentification", AN);
						addLeafNode(new SimpleItem("RecordCategoryCode", AN, FieldID::TYPE_1_CNT, SubfieldIndex(1), ItemIndex(0)));
					finishNode("ContentRecordSummary", AN);
				finishNode("TransactionContentSummary", AN);

				addNode(new SubfieldIncrementer("TransactionCharacterSetDirectory", AN));
					addLeafNode(new SimpleItem("CharacterSetCommonNameCode", AN, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("CharacterSetIndexCode", AN, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("CharacterSetVersionIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_1_DCS, SubfieldIndex(0), ItemIndex(2)));
					finishNode("CharacterSetVersionIdentification", AN);
				finishNode("TransactionCharacterSetDirectory", AN);

				addNode(new Converter("TransactionAugmentation", EBTS));
					addLeafNode(new SimpleField("TransactionCategoryCode", EBTS, FieldID::TYPE_1_TOT));
				finishNode("TransactionAugmentation", EBTS);
			finishNode("Transaction", EBTS);
		finishNode("PackageInformationRecord", ITL);
	}

	void RecordConverter::configureType2AConverter() {
		addNode(new Converter("PackageDescriptiveTextRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "02"));

			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_2_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);
		finishNode("PackageDescriptiveTextRecord", ITL);
	}

	void RecordConverter::configureType2BConverter() {
		addNode(new Converter("PackageDescriptiveTextRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "02"));

			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_2_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("UserDefinedDescriptiveText", ITL));
				addNode(new Converter("DomainDefinedDescriptiveFields", EBTS));
					addNode(new SubfieldIncrementer("RecordForwardOrganizations", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleSubfield("IdentificationID", NC, FieldID::TYPE_2_SCO, SubfieldIndex(0)));
						finishNode("OrganizationIdentification", NC);
					finishNode("RecordForwardOrganizations", AN);

					addNode(new Converter("RecordTransactionData", EBTS));
						addLeafNode(new SimpleField("TransactionQueryDepthCode", EBTS, FieldID::TYPE_2_QDD));
					finishNode("RecordTransactionData", EBTS);

					addNode(new Converter("RecordActivity", EBTS));
						addNode(new SubfieldIncrementer("ContributorCaseIdentificationNumber", EBTS));
							addNode(new Converter("ContributorCasePrefixIdentification", EBTS));
								addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_2_CIN, SubfieldIndex(0), ItemIndex(0)));
							finishNode("ContributorCasePrefixIdentification", EBTS);
							addNode(new Converter("ContributorCaseIdentification", EBTS));
								addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_2_CIN, SubfieldIndex(0), ItemIndex(1)));
							finishNode("ContributorCaseIdentification", EBTS);
							addNode(new Converter("ContributorCaseExtensionIdentification", EBTS));
								addLeafNode(new SimpleSubfield("IdentificationID", NC, FieldID::TYPE_2_CIX, SubfieldIndex(0)));
							finishNode("ContributorCaseExtensionIdentification", EBTS);
						finishNode("ContributorCaseIdentificationNumber", EBTS);

						addNode(new Converter("FBIFileNumber", EBTS));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_2_FFN));
						finishNode("FBIFileNumber", EBTS);

						addNode(new Converter("FBILatentCaseIdentification", EBTS));
							addNode(new Converter("FBILatentCaseNumber", EBTS));
								addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_2_LCN));
							finishNode("FBILatentCaseNumber", EBTS);
							addNode(new Converter("FBILatentCaseNumberExtension", EBTS));
								addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_2_LCX));
							finishNode("FBILatentCaseNumberExtension", EBTS);
						finishNode("FBILatentCaseIdentification", EBTS);
					finishNode("RecordActivity", EBTS);

					addNode(new Converter("RecordSubject", EBTS));
						addNode(new SubfieldIncrementer("PersonFBIIdentification", J));
							addLeafNode(new SimpleSubfield("IdentificationID", NC, FieldID::TYPE_2_FBI, SubfieldIndex(0)));
						finishNode("PersonFBIIdentification", J);
					finishNode("RecordSubject", EBTS);
				finishNode("DomainDefinedDescriptiveFields", EBTS);
			finishNode("UserDefinedDescriptiveText", ITL);

		finishNode("PackageDescriptiveTextRecord", ITL);
	}

	void RecordConverter::configureType3Converter() {
		addNode(new Converter("PackageLowResolutionGrayscaleImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "03"));

			configureType3_6Converter(TYPE3);

		finishNode("PackageLowResolutionGrayscaleImageRecord", ITL);
	}

	void RecordConverter::configureType4Converter() {
		addNode(new Converter("PackageHighResolutionGrayscaleImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "04"));

			configureType3_6Converter(TYPE4);

		finishNode("PackageHighResolutionGrayscaleImageRecord", ITL);
	}

	void RecordConverter::configureType5Converter() {
		addNode(new Converter("PackageLowResolutionBinaryImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "05"));

			configureType3_6Converter(TYPE5);

		finishNode("PackageLowResolutionBinaryImageRecord", ITL);
	}

	void RecordConverter::configureType6Converter() {
		addNode(new Converter("PackageHighResolutionBinaryImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "06"));

			configureType3_6Converter(TYPE6);

		finishNode("PackageHighResolutionBinaryImageRecord", ITL);
	}

	void RecordConverter::configureType3_6Converter(RecordType recordType) {
		addNode(new Converter("ImageReferenceIdentification", AN));
			addLeafNode(new SimpleField("IdentificationID", NC, FieldID(recordType, 2), new IDCFormatter));
		finishNode("ImageReferenceIdentification", AN);

		addNode(new Converter("FingerprintImage", AN));
			addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID(recordType, 9), new Base64Formatter));

			addNode(new Converter("ImageCaptureDetail", AN));
				addLeafNode(new SimpleField("CaptureResolutionCode", AN, FieldID(recordType, 5)));
			finishNode("ImageCaptureDetail", AN);

			addLeafNode(new SimpleField("ImageCompressionAlgorithmCode", AN, FieldID(recordType, 8)));
			addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID(recordType, 6)));
			addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID(recordType, 7)));

			addNode(new Converter("FingerprintImagePosition", AN));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID(recordType, 4), SubfieldIndex(0)));
			finishNode("FingerprintImagePosition", AN);

			addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID(recordType, 3)));

		finishNode("FingerprintImage", AN);
	}

	void RecordConverter::configureType7Converter() {
		addNode(new Converter("PackageUserDefinedImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "07"));

			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_7_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);
		finishNode("PackageUserDefinedImageRecord", ITL);
	}

	void RecordConverter::configureType8AConverter() {
		addNode(new Converter("PackageSignatureImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "08"));

			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_8_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("SignatureImage", AN));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_8_DATA, new Base64Formatter));

				addNode(new Converter("ImageCaptureDetail", AN));
					addLeafNode(new SimpleField("CaptureResolutionCode", AN, FieldID::TYPE_8_ISR));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_8_HLL));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_8_VLL));
				addLeafNode(new SimpleField("SignatureRepresentationCode", AN, FieldID::TYPE_8_SRT));
				addLeafNode(new SimpleField("SignatureCategoryCode", AN, FieldID::TYPE_8_SIG));

			finishNode("SignatureImage", AN);
		finishNode("PackageSignatureImageRecord", ITL);
	}

	void RecordConverter::configureType8BConverter() {
		addNode(new Converter("PackageSignatureImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "08"));

			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_8_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("SignatureImage", AN));
				addNode(new Converter("ImageCaptureDetail", AN));
					addLeafNode(new SimpleField("CaptureResolutionCode", AN, FieldID::TYPE_8_ISR));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_8_HLL, new Type8LLFormatter));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_8_VLL, new Type8LLFormatter));

				addNode(new Converter("SignatureImageVectorRepresentation", AN));
					addNode(new SubfieldIncrementer("SignatureImageVector", AN));
						addLeafNode(new SimpleItem("VectorPenPressureValue", AN, FieldID::TYPE_8_DATA, SubfieldIndex(0), ItemIndex(2)));
						addLeafNode(new SimpleItem("VectorPositionVerticalCoordinateValue", AN, FieldID::TYPE_8_DATA, SubfieldIndex(0), ItemIndex(1)));
						addLeafNode(new SimpleItem("VectorPositionHorizontalCoordinateValue", AN, FieldID::TYPE_8_DATA, SubfieldIndex(0), ItemIndex(0)));
					finishNode("SignatureImageVector", AN);
				finishNode("SignatureImageVectorRepresentation", AN);

				addLeafNode(new SimpleField("SignatureRepresentationCode", AN, FieldID::TYPE_8_SRT));
				addLeafNode(new SimpleField("SignatureCategoryCode", AN, FieldID::TYPE_8_SIG));

			finishNode("SignatureImage", AN);
		finishNode("PackageSignatureImageRecord", ITL);
	}

	void RecordConverter::configureType9AConverter() {
		addNode(new Converter("PackageMinutiaeRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "09"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_9_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addLeafNode(new SimpleField("MinutiaeImpressionCaptureCategoryCode", AN, FieldID::TYPE_9_IMP));
			addLeafNode(new SimpleField("MinutiaeFormatNISTStandardIndicator", AN, FieldID::TYPE_9_FMT, new StandardIndicatorFormatter));

			addNode(new Converter("Minutiae", ITL));
				addNode(new Converter("MinutiaeNISTStandard", ITL));
					addNode(new SubfieldIncrementer("MinutiaDetail", ITL));
						addLeafNode(new CompositeItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(0), SubstrLen(4)));
						addLeafNode(new CompositeItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(4), SubstrLen(4)));
						addNode(new Converter("MinutiaIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(0)));
						finishNode("MinutiaIdentification", AN);
						addLeafNode(new CompositeItem("PositionThetaAngleMeasure", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(8), SubstrLen(3)));
						addLeafNode(new SimpleItem("MinutiaQualityValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(2)));
						addLeafNode(new SimpleItem("MinutiaCategoryCode", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(3)));
						addNode(new ItemIncrementer("MinutiaRidgeCount", AN));
							addNode(new Converter("RidgeCountReferenceIdentification", AN));
								addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(4), ItemStep(1), new ItemSplitFormatter(',', 0)));
							finishNode("RidgeCountReferenceIdentification", AN);
							addLeafNode(new SimpleItem("RidgeCountValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(4), ItemStep(1), new ItemSplitFormatter(',', 1)));
						finishNode("MinutiaRidgeCount", AN);
					finishNode("MinutiaDetail", ITL);

					addLeafNode(new SimpleField("MinutiaeQuantity", AN, FieldID::TYPE_9_MIN));

					addNode(new Converter("MinutiaeReadingSystem", AN));
						addLeafNode(new SimpleItem("ReadingSystemCodingMethodCode", AN, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(1)));
						addLeafNode(new SimpleItem("ReadingSystemName", AN, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(0)));
						addNode(new Converter("ReadingSystemSubsystemIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(2)));
						finishNode("ReadingSystemSubsystemIdentification", AN);
					finishNode("MinutiaeReadingSystem", AN);

					addLeafNode(new SimpleField("MinutiaeRidgeCountIndicator", AN, FieldID::TYPE_9_RDG, new RidgeCountIndicatorFormatter));
				finishNode("MinutiaeNISTStandard", ITL);

				addNode(new SubfieldIncrementer("MinutiaeFingerCorePosition", AN));
					addLeafNode(new CompositeItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_CRP, SubfieldIndex(0), ItemIndex(0), SubstrIndex(0), SubstrLen(4)));
					addLeafNode(new CompositeItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_CRP, SubfieldIndex(0), ItemIndex(0), SubstrIndex(4), SubstrLen(4)));
				finishNode("MinutiaeFingerCorePosition", AN);

				addNode(new SubfieldIncrementer("MinutiaeFingerDeltaPosition", AN));
					addLeafNode(new CompositeItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_DLT, SubfieldIndex(0), ItemIndex(0), SubstrIndex(0), SubstrLen(4)));
					addLeafNode(new CompositeItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_DLT, SubfieldIndex(0), ItemIndex(0), SubstrIndex(4), SubstrLen(4)));
				finishNode("MinutiaeFingerDeltaPosition", AN);

				addNode(new Converter("MinutiaeFingerPatternDetail", ITL));
					addLeafNode(new SimpleItem("FingerPatternCodeSourceCode", ITL, FieldID::TYPE_9_FPC, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("FingerPatternCode", AN, FieldID::TYPE_9_FPC, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleListSubfield("FingerPatternCode", AN, FieldID::TYPE_9_FPC, SubfieldIndex(1)));
				finishNode("MinutiaeFingerPatternDetail", ITL);

				addLeafNode(new SimpleListSubfield("MinutiaeFingerPositionCode", AN, FieldID::TYPE_9_FGP, SubfieldIndex(0)));
			finishNode("Minutiae", ITL);
		finishNode("PackageMinutiaeRecord", ITL);
	}

	void RecordConverter::configureType9BConverter() {
		addNode(new Converter("PackageMinutiaeRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "09"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_9_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addLeafNode(new SimpleField("MinutiaeImpressionCaptureCategoryCode", AN, FieldID::TYPE_9_IMP));
			addLeafNode(new SimpleField("MinutiaeFormatNISTStandardIndicator", AN, FieldID::TYPE_9_FMT, new StandardIndicatorFormatter));

			addNode(new Converter("Minutiae", ITL));
				addNode(new Converter("MinutiaeNISTStandard", ITL));
					addNode(new SubfieldIncrementer("MinutiaDetail", ITL));
						addLeafNode(new CompositeItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(0), SubstrLen(5)));
						addLeafNode(new CompositeItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(5), SubstrLen(5)));
						addNode(new Converter("MinutiaIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(0)));
						finishNode("MinutiaIdentification", AN);
						addLeafNode(new CompositeItem("PositionThetaAngleMeasure", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(1), SubstrIndex(10), SubstrLen(3)));
						addLeafNode(new SimpleItem("MinutiaQualityValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(2)));
						addLeafNode(new SimpleItem("MinutiaCategoryCode", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(3)));
						addNode(new ItemIncrementer("MinutiaRidgeCount", AN));
							addNode(new Converter("RidgeCountReferenceIdentification", AN));
								addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(4), ItemStep(1), new ItemSplitFormatter(',', 0)));
							finishNode("RidgeCountReferenceIdentification", AN);
							addLeafNode(new SimpleItem("RidgeCountValue", AN, FieldID::TYPE_9_MRC, SubfieldIndex(0), ItemIndex(4), ItemStep(1), new ItemSplitFormatter(',', 1)));
						finishNode("MinutiaRidgeCount", AN);
					finishNode("MinutiaDetail", ITL);

					addLeafNode(new SimpleField("MinutiaeQuantity", AN, FieldID::TYPE_9_MIN));

					addNode(new Converter("MinutiaeReadingSystem", AN));
						addLeafNode(new SimpleItem("ReadingSystemCodingMethodCode", AN, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(1)));
						addLeafNode(new SimpleItem("ReadingSystemName", AN, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(0)));
						addNode(new Converter("ReadingSystemSubsystemIdentification", AN));
							addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_OFR, SubfieldIndex(0), ItemIndex(2)));
						finishNode("ReadingSystemSubsystemIdentification", AN);
					finishNode("MinutiaeReadingSystem", AN);

					addLeafNode(new SimpleField("MinutiaeRidgeCountIndicator", AN, FieldID::TYPE_9_RDG, new RidgeCountIndicatorFormatter));
				finishNode("MinutiaeNISTStandard", ITL);

				addLeafNode(new SimpleListSubfield("MinutiaePalmPositionCode", AN, FieldID::TYPE_9_FGP, SubfieldIndex(0)));
			finishNode("Minutiae", ITL);
		finishNode("PackageMinutiaeRecord", ITL);
	}

	void RecordConverter::configureType9CConverter() {
		addNode(new Converter("PackageMinutiaeRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "09"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_9_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addLeafNode(new SimpleField("MinutiaeImpressionCaptureCategoryCode", AN, FieldID::TYPE_9_IMP));
			addLeafNode(new SimpleField("MinutiaeFormatNISTStandardIndicator", AN, FieldID::TYPE_9_FMT, new StandardIndicatorFormatter));
		finishNode("PackageMinutiaeRecord", ITL);
	}

	void RecordConverter::configureType9DConverter() {
		addNode(new Converter("PackageMinutiaeRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "09"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_9_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addLeafNode(new SimpleField("MinutiaeImpressionCaptureCategoryCode", AN, FieldID::TYPE_9_IMP));
			addLeafNode(new SimpleField("MinutiaeFormatNISTStandardIndicator", AN, FieldID::TYPE_9_FMT, new StandardIndicatorFormatter));

			addNode(new Converter("Minutiae", INCITS));
				addNode(new Converter("CBEFFFormatOwnerIdentification", AN));
					addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_126, SubfieldIndex(0), ItemIndex(2)));
				finishNode("CBEFFFormatOwnerIdentification", AN);

				addNode(new Converter("CBEFFFormatCategoryIdentification", AN));
					addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_126, SubfieldIndex(0), ItemIndex(0)));
				finishNode("CBEFFFormatCategoryIdentification", AN);

				addNode(new Converter("CBEFFProductIdentification", INCITS));
					addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_126, SubfieldIndex(0), ItemIndex(1)));
				finishNode("CBEFFProductIdentification", INCITS);

				addNode(new Converter("FingerprintImageCapture", INCITS));
					addLeafNode(new SimpleItem("CaptureDeviceCertificationText", INCITS, FieldID::TYPE_9_127, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new Converter("CaptureDeviceIdentification", INCITS));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_127, SubfieldIndex(0), ItemIndex(1)));
					finishNode("CaptureDeviceIdentification", INCITS);
				finishNode("FingerprintImageCapture", INCITS);

				addNode(new Converter("NISTImage", AN));
					addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_9_128));
					addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_9_131));
					addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_9_130));
					addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_9_129));
					addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_9_132));
				finishNode("NISTImage", AN);

				addLeafNode(new SimpleField("FingerViewCode", INCITS, FieldID::TYPE_9_133));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", INCITS, FieldID::TYPE_9_134, SubfieldIndex(0)));

				addNode(new SubfieldIncrementer("MinutiaeQuality", INCITS));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_135, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_9_135, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityMeasureVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_135, SubfieldIndex(0), ItemIndex(1)));
					finishNode("QualityMeasureVendorIdentification", AN);
				finishNode("MinutiaeQuality", INCITS);

				addLeafNode(new SimpleField("MinutiaeQuantity", AN, FieldID::TYPE_9_136));

				addNode(new SubfieldIncrementer("MinutiaDetail", ITL));
					addLeafNode(new SimpleItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(2)));
					addNode(new Converter("MinutiaIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(0)));
					finishNode("MinutiaIdentification", AN);
					addLeafNode(new SimpleItem("PositionThetaAngleMeasure", AN, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(3)));
					addLeafNode(new SimpleItem("MinutiaQualityValue", AN, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(5)));
					addLeafNode(new SimpleItem("MinutiaCategoryCode", INCITS, FieldID::TYPE_9_137, SubfieldIndex(0), ItemIndex(4)));
				finishNode("MinutiaDetail", ITL);

				addLeafNode(new SimpleItem("RidgeCountExtractionMethodCode", INCITS, FieldID::TYPE_9_138, SubfieldIndex(0), ItemIndex(0)));

				addNode(new SubfieldIncrementer("RidgeCountDetails", INCITS));
					addNode(new Converter("MinutiaIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_138, SubfieldIndex(1), ItemIndex(0)));
					finishNode("MinutiaIdentification", AN);
					addNode(new Converter("RidgeCountReferenceIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_9_138, SubfieldIndex(1), ItemIndex(1)));
					finishNode("RidgeCountReferenceIdentification", AN);
					addLeafNode(new SimpleItem("RidgeCountValue", AN, FieldID::TYPE_9_138, SubfieldIndex(1), ItemIndex(2)));
				finishNode("RidgeCountDetails", INCITS);

				addNode(new SubfieldIncrementer("MinutiaeFingerCorePosition", INCITS));
					addLeafNode(new SimpleItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_139, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_139, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("PositionThetaAngleMeasure", AN, FieldID::TYPE_9_139, SubfieldIndex(0), ItemIndex(2)));
				finishNode("MinutiaeFingerCorePosition", INCITS);

				addNode(new SubfieldIncrementer("MinutiaeFingerDeltaPosition", INCITS));
					addLeafNode(new SimpleItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_9_140, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_9_140, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("PositionThetaAngleMeasure", AN, FieldID::TYPE_9_140, SubfieldIndex(0), ItemIndex(2)));
				finishNode("MinutiaeFingerDeltaPosition", INCITS);
			finishNode("Minutiae", INCITS);
		finishNode("PackageMinutiaeRecord", ITL);
	}

	void RecordConverter::configureType10AConverter() {
		addNode(new Converter("PackageFacialAndSMTImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "10"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_10_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("PhysicalFeatureImage", AN));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_10_DATA, new Base64Formatter));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_10_PHD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_10_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_10_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_10_SVPS));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageColorSpaceCode", AN, FieldID::TYPE_10_CSP));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_10_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_10_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_10_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_10_SLC));
				addLeafNode(new SimpleField("ImageCategoryCode", AN, FieldID::TYPE_10_IMT));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_10_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_10_VPS));

				addNode(new SubfieldIncrementer("PhysicalFeatureDescriptionDetail", AN));
					addNode(new Converter("PhysicalFeatureColorDetail", AN));
						addLeafNode(new SimpleItem("PhysicalFeaturePrimaryColorCode", AN, FieldID::TYPE_10_COL, SubfieldIndex(0), ItemIndex(0)));
						addLeafNode(new SimpleListItem("PhysicalFeatureSecondaryColorCode", AN, FieldID::TYPE_10_COL, SubfieldIndex(0), ItemIndex(1), ItemStep(1)));
					finishNode("PhysicalFeatureColorDetail", AN);
					addLeafNode(new SimpleItem("PhysicalFeatureCategoryCode", AN, FieldID::TYPE_10_SMD, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PhysicalFeatureClassCode", AN, FieldID::TYPE_10_SMD, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("PhysicalFeatureDescriptionText", AN, FieldID::TYPE_10_SMD, SubfieldIndex(0), ItemIndex(3)));
					addLeafNode(new SimpleItem("PhysicalFeatureSubClassCode", AN, FieldID::TYPE_10_SMD, SubfieldIndex(0), ItemIndex(2)));
				finishNode("PhysicalFeatureDescriptionDetail", AN);

				addLeafNode(new SimpleListSubfield("PhysicalFeatureNCICCode", AN, FieldID::TYPE_10_SMT, SubfieldIndex(0)));

				addNode(new Converter("PhysicalFeatureSize", AN));
					addLeafNode(new SimpleItem("PhysicalFeatureHeightMeasure", AN, FieldID::TYPE_10_SMS, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PhysicalFeatureWidthMeasure", AN, FieldID::TYPE_10_SMS, SubfieldIndex(0), ItemIndex(1)));
				finishNode("PhysicalFeatureSize", AN);
			finishNode("PhysicalFeatureImage", AN);
		finishNode("PackageFacialAndSMTImageRecord", ITL);
	}

	void RecordConverter::configureType10BConverter() {
		addNode(new Converter("PackageFacialAndSMTImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "10"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_10_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("FaceImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_10_DATA, new Base64Formatter));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_10_PHD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_10_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_10_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_10_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_10_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageColorSpaceCode", AN, FieldID::TYPE_10_CSP));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_10_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_10_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_10_HPS));

				addNode(new SubfieldIncrementer("ImageQuality", AN));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_10_SQS, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_10_SQS, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityMeasureVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_10_SQS, SubfieldIndex(0), ItemIndex(1)));
					finishNode("QualityMeasureVendorIdentification", AN);
				finishNode("ImageQuality", AN);

				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_10_SLC));
				addLeafNode(new SimpleField("ImageCategoryCode", AN, FieldID::TYPE_10_IMT));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_10_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_10_VPS));

				addNode(new Converter("FaceImage3DPoseAngle", AN));
					addLeafNode(new SimpleItem("PosePitchAngleMeasure", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("PosePitchUncertaintyValue", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(4)));
					addLeafNode(new SimpleItem("PoseRollAngleMeasure", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(2)));
					addLeafNode(new SimpleItem("PoseRollUncertaintyValue", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(5)));
					addLeafNode(new SimpleItem("PoseYawAngleMeasure", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PoseYawUncertaintyValue", AN, FieldID::TYPE_10_SPA, SubfieldIndex(0), ItemIndex(3)));
				finishNode("FaceImage3DPoseAngle", AN);

				addLeafNode(new SimpleField("FaceImageAcquisitionProfileCode", AN, FieldID::TYPE_10_SAP));

				addNode(new SubfieldIncrementer("FaceImageAttribute", AN));
					addLeafNode(new SimpleItem("FaceImageAttributeCode", AN, FieldID::TYPE_10_PXS, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("FaceImageAttributeText", AN, FieldID::TYPE_10_PXS, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FaceImageAttribute", AN);

				addLeafNode(new SimpleListSubfield("FaceImageDescriptionCode", AN, FieldID::TYPE_10_SXS, SubfieldIndex(0)));
				addLeafNode(new SimpleField("FaceImageEyeColorAttributeCode", AN, FieldID::TYPE_10_SEC));

				addNode(new SubfieldIncrementer("FaceImageFeaturePoint", AN));
					addLeafNode(new SimpleItem("FeaturePointHorizontalCoordinateValue", AN, FieldID::TYPE_10_SFP, SubfieldIndex(0), ItemIndex(2)));
					addNode(new Converter("FeaturePointIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_10_SFP, SubfieldIndex(0), ItemIndex(1)));
					finishNode("FeaturePointIdentification", AN);
					addLeafNode(new SimpleItem("FeaturePointCategoryCode", AN, FieldID::TYPE_10_SFP, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("FeaturePointVerticalCoordinateValue", AN, FieldID::TYPE_10_SFP, SubfieldIndex(0), ItemIndex(3)));
				finishNode("FaceImageFeaturePoint", AN);

				addLeafNode(new SimpleListSubfield("FaceImageHairColorAttributeCode", AN, FieldID::TYPE_10_SHC, SubfieldIndex(0)));
				addLeafNode(new SimpleField("FaceImagePoseOffsetAngleMeasure", AN, FieldID::TYPE_10_POA));
				addLeafNode(new SimpleField("FaceImageSubjectPoseCode", AN, FieldID::TYPE_10_POS));

				addNode(new Converter("FaceImageAcquisitionSource", ITL));
					addLeafNode(new SimpleItem("CaptureSourceCode", AN, FieldID::TYPE_10_PAS, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("CaptureSourceDescriptionText", ITL, FieldID::TYPE_10_PAS, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FaceImageAcquisitionSource", ITL);
			finishNode("FaceImage", ITL);
		finishNode("PackageFacialAndSMTImageRecord", ITL);
	}

	void RecordConverter::configureType13AConverter() {
		addNode(new Converter("PackageLatentImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "13"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("FingerprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_13_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_13_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_13_LCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_13_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_13_SVPS));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_13_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_13_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_13_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_13_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_13_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_13_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_13_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_13_IMP));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID::TYPE_13_FGP, SubfieldIndex(0)));

				addNode(new SubfieldIncrementer("FingerprintImageQuality", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageQuality", ITL);

			finishNode("FingerprintImage", ITL);
		finishNode("PackageLatentImageRecord", ITL);
	}

	void RecordConverter::configureType13BConverter() {
		addNode(new Converter("PackageLatentImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "13"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("FingerprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_13_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_13_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_13_LCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_13_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_13_SVPS));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_13_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_13_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_13_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_13_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_13_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_13_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_13_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_13_IMP));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID::TYPE_13_FGP, SubfieldIndex(0)));

				addNode(new Converter("FingerprintImageMajorCasePrint", AN));
					addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID::TYPE_13_SPD, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleListSubfield("MajorCasePrintCode", AN, FieldID::TYPE_13_SPD, SubfieldIndex(0), ItemIndex(1)));
					addNode(new SubfieldIncrementer("MajorCasePrintSegmentOffset", AN));
						addLeafNode(new SimpleItem("SegmentBottomVerticalCoordinateValue", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(5)));
						addLeafNode(new SimpleItem("SegmentLocationCode", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(1)));
						addLeafNode(new SimpleItem("SegmentFingerViewCode", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(0)));
						addLeafNode(new SimpleItem("SegmentLeftHorizontalCoordinateValue", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(2)));
						addLeafNode(new SimpleItem("SegmentRightHorizontalCoordinateValue", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(3)));
						addLeafNode(new SimpleItem("SegmentTopVerticalCoordinateValue", AN, FieldID::TYPE_13_PPC, SubfieldIndex(0), ItemIndex(4)));
					finishNode("MajorCasePrintSegmentOffset", AN);
				finishNode("FingerprintImageMajorCasePrint", AN);

				addNode(new SubfieldIncrementer("FingerprintImageQuality", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageQuality", ITL);

			finishNode("FingerprintImage", ITL);
		finishNode("PackageLatentImageRecord", ITL);
	}

	void RecordConverter::configureType13CConverter() {
		addNode(new Converter("PackageLatentImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "13"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("PalmprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_13_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_13_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_13_LCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_13_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_13_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_13_SVPS));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_13_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_13_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_13_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_13_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_13_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_13_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_13_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_13_IMP));
				addLeafNode(new SimpleListSubfield("PalmPositionCode", AN, FieldID::TYPE_13_FGP, SubfieldIndex(0)));

				addNode(new SubfieldIncrementer("PalmprintImageQuality", ITL));
					addLeafNode(new SimpleItem("PalmPositionCode", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_13_LQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("PalmprintImageQuality", ITL);

			finishNode("PalmprintImage", ITL);
		finishNode("PackageLatentImageRecord", ITL);
	}

	void RecordConverter::configureType14AConverter() {
		addNode(new Converter("PackageFingerprintImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "14"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_14_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("FingerprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_14_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_14_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_14_FCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_14_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_14_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_14_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_14_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_14_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_14_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_14_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_14_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_14_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_14_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_14_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_14_IMP));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID::TYPE_14_FGP, SubfieldIndex(0)));

				addNode(new SubfieldIncrementer("FingerprintImageFingerMissing", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_AMP, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("FingerMissingCode", ITL, FieldID::TYPE_14_AMP, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageFingerMissing", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentPositionSquare", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("SegmentBottomVerticalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(4)));
					addLeafNode(new SimpleItem("SegmentLeftHorizontalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("SegmentRightHorizontalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(2)));
					addLeafNode(new SimpleItem("SegmentTopVerticalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(3)));
				finishNode("FingerprintImageSegmentPositionSquare", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageNISTQuality", AN));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_NQM, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("NISTQualityMeasure", AN, FieldID::TYPE_14_NQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageNISTQuality", AN);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentationQuality", AN));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageSegmentationQuality", AN);

				addNode(new SubfieldIncrementer("FingerprintImageQuality", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageQuality", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentPositionPolygon", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PositionPolygonVertexQuantity", ITL, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(1)));
					addNode(new ItemIncrementer("PositionPolygonVertex", ITL));
						addLeafNode(new SimpleItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(2), ItemStep(2)));
						addLeafNode(new SimpleItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(3), ItemStep(2)));
					finishNode("PositionPolygonVertex", ITL);
				finishNode("FingerprintImageSegmentPositionPolygon", ITL);

			finishNode("FingerprintImage", ITL);
		finishNode("PackageFingerprintImageRecord", ITL);
	}

	void RecordConverter::configureType14BConverter() {
		addNode(new Converter("PackageFingerprintImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "14"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_14_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("FingerprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_14_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_14_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_14_FCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_14_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_14_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_14_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_14_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_14_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_14_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_14_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_14_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_14_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_14_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_14_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_14_IMP));
				addLeafNode(new SimpleListSubfield("FingerPositionCode", AN, FieldID::TYPE_14_FGP, SubfieldIndex(0)));

				addNode(new Converter("FingerprintImageMajorCasePrint", AN));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_PPD, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("MajorCasePrintCode", AN, FieldID::TYPE_14_PPD, SubfieldIndex(0), ItemIndex(1)));
					addNode(new SubfieldIncrementer("MajorCasePrintSegmentOffset", AN));
						addLeafNode(new SimpleItem("SegmentBottomVerticalCoordinateValue", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(5)));
						addLeafNode(new SimpleItem("SegmentLocationCode", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(1)));
						addLeafNode(new SimpleItem("SegmentFingerViewCode", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(0)));
						addLeafNode(new SimpleItem("SegmentLeftHorizontalCoordinateValue", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(2)));
						addLeafNode(new SimpleItem("SegmentRightHorizontalCoordinateValue", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(3)));
						addLeafNode(new SimpleItem("SegmentTopVerticalCoordinateValue", AN, FieldID::TYPE_14_PPC, SubfieldIndex(0), ItemIndex(4)));
					finishNode("MajorCasePrintSegmentOffset", AN);
				finishNode("FingerprintImageMajorCasePrint", AN);

				addNode(new SubfieldIncrementer("FingerprintImageFingerMissing", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_AMP, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("FingerMissingCode", ITL, FieldID::TYPE_14_AMP, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageFingerMissing", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentPositionSquare", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("SegmentBottomVerticalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(4)));
					addLeafNode(new SimpleItem("SegmentLeftHorizontalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("SegmentRightHorizontalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(2)));
					addLeafNode(new SimpleItem("SegmentTopVerticalCoordinateValue", AN, FieldID::TYPE_14_SEG, SubfieldIndex(0), ItemIndex(3)));
				finishNode("FingerprintImageSegmentPositionSquare", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageNISTQuality", AN));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_NQM, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("NISTQualityMeasure", AN, FieldID::TYPE_14_NQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageNISTQuality", AN);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentationQuality", AN));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_14_SQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageSegmentationQuality", AN);

				addNode(new SubfieldIncrementer("FingerprintImageQuality", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_14_FQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("FingerprintImageQuality", ITL);

				addNode(new SubfieldIncrementer("FingerprintImageSegmentPositionPolygon", ITL));
					addLeafNode(new SimpleItem("FingerPositionCode", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("PositionPolygonVertexQuantity", ITL, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(1)));
					addNode(new ItemIncrementer("PositionPolygonVertex", ITL));
						addLeafNode(new SimpleItem("PositionHorizontalCoordinateValue", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(2), ItemStep(2)));
						addLeafNode(new SimpleItem("PositionVerticalCoordinateValue", AN, FieldID::TYPE_14_ASEG, SubfieldIndex(0), ItemIndex(3), ItemStep(2)));
					finishNode("PositionPolygonVertex", ITL);
				finishNode("FingerprintImageSegmentPositionPolygon", ITL);

			finishNode("FingerprintImage", ITL);
		finishNode("PackageFingerprintImageRecord", ITL);
	}

	void RecordConverter::configureType15Converter() {
		addNode(new Converter("PackagePalmprintImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "15"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_15_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("PalmprintImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_15_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_15_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_15_PCD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_15_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_15_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_15_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_15_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_15_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_15_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_15_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_15_HPS));
				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_15_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_15_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_15_VPS));
				addLeafNode(new SimpleField("FingerprintImageImpressionCaptureCategoryCode", AN, FieldID::TYPE_15_IMP));
				addLeafNode(new SimpleField("PalmPositionCode", AN, FieldID::TYPE_15_PLP));

				addNode(new SubfieldIncrementer("PalmprintImageQuality", ITL));
					addLeafNode(new SimpleItem("PalmPositionCode", AN, FieldID::TYPE_15_PQM, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_15_PQM, SubfieldIndex(0), ItemIndex(3)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addNode(new Converter("QualityAlgorithmVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_15_PQM, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmVendorIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_15_PQM, SubfieldIndex(0), ItemIndex(1)));
				finishNode("PalmprintImageQuality", ITL);

			finishNode("PalmprintImage", ITL);
		finishNode("PackagePalmprintImageRecord", ITL);
	}

	void RecordConverter::configureType16Converter() {
		addNode(new Converter("PackageUserDefinedTestingImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "16"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_16_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("TestImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_16_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("BinaryDescriptionText", NC, FieldID::TYPE_16_UDI));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_16_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_16_UTD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_16_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_16_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_16_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_16_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageColorSpaceCode", AN, FieldID::TYPE_16_CSP));
				addLeafNode(new SimpleField("ImageCommentText", AN, FieldID::TYPE_16_COM));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_16_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_16_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_16_HPS));

				addNode(new SubfieldIncrementer("ImageQuality", AN));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_16_UQS, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_16_UQS, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityMeasureVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_16_UQS, SubfieldIndex(0), ItemIndex(1)));
					finishNode("QualityMeasureVendorIdentification", AN);
				finishNode("ImageQuality", AN);

				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_16_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_16_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_16_VPS));
			finishNode("TestImage", ITL);
		finishNode("PackageUserDefinedTestingImageRecord", ITL);
	}

	void RecordConverter::configureType17Converter() {
		addNode(new Converter("PackageIrisImageRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "17"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_17_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("IrisImage", ITL));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_17_DATA, new Base64Formatter));
				addLeafNode(new SimpleField("ImageBitsPerPixelQuantity", AN, FieldID::TYPE_17_BPX));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("Date", NC, FieldID::TYPE_17_ICD, new DateFormatter));
					finishNode("CaptureDate", AN);
					addLeafNode(new SimpleField("CaptureHorizontalPixelDensityValue", AN, FieldID::TYPE_17_SHPS));
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_17_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
					addLeafNode(new SimpleField("CaptureVerticalPixelDensityValue", AN, FieldID::TYPE_17_SVPS));
					addLeafNode(new SimpleField("CaptureDeviceMonitoringModeCode", AN, FieldID::TYPE_17_DMM));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new SimpleField("ImageColorSpaceCode", AN, FieldID::TYPE_17_CSP));
				addLeafNode(new SimpleListSubfield("ImageCommentText", AN, FieldID::TYPE_17_COM, SubfieldIndex(0)));
				addLeafNode(new SimpleField("ImageCompressionAlgorithmText", AN, FieldID::TYPE_17_CGA));
				addLeafNode(new SimpleField("ImageHorizontalLineLengthPixelQuantity", AN, FieldID::TYPE_17_HLL));
				addLeafNode(new SimpleField("ImageHorizontalPixelDensityValue", AN, FieldID::TYPE_17_HPS));

				addNode(new SubfieldIncrementer("ImageQuality", AN));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_17_IQS, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_17_IQS, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityMeasureVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_17_IQS, SubfieldIndex(0), ItemIndex(1)));
					finishNode("QualityMeasureVendorIdentification", AN);
				finishNode("ImageQuality", AN);

				addLeafNode(new SimpleField("ImageScaleUnitsCode", AN, FieldID::TYPE_17_SLC));
				addLeafNode(new SimpleField("ImageVerticalLineLengthPixelQuantity", AN, FieldID::TYPE_17_VLL));
				addLeafNode(new SimpleField("ImageVerticalPixelDensityValue", AN, FieldID::TYPE_17_VPS));
				addLeafNode(new SimpleField("IrisEyePositionCode", AN, FieldID::TYPE_17_FID));
				addLeafNode(new SimpleField("IrisEyeRotationAngleMeasure", AN, FieldID::TYPE_17_RAE));
				addLeafNode(new SimpleField("IrisEyeRotationUncertaintyValueText", AN, FieldID::TYPE_17_RAU));

				addNode(new Converter("IrisImageCapture", AN));
					addNode(new Converter("CaptureDeviceGlobalIdentification", AN));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_17_GUI));
					finishNode("CaptureDeviceGlobalIdentification", AN);
					addNode(new Converter("CaptureDeviceIdentification", AN));
						addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_17_DUI));
					finishNode("CaptureDeviceIdentification", AN);
					addLeafNode(new SimpleItem("CaptureDeviceMakeText", AN, FieldID::TYPE_17_MMS, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("CaptureDeviceModelText", AN, FieldID::TYPE_17_MMS, SubfieldIndex(0), ItemIndex(1)));
					addLeafNode(new SimpleItem("CaptureDeviceSerialNumberText", AN, FieldID::TYPE_17_MMS, SubfieldIndex(0), ItemIndex(2)));
					addLeafNode(new SimpleItem("IrisImageHorizontalOrientationCode", AN, FieldID::TYPE_17_IPC, SubfieldIndex(0), ItemIndex(0)));
					addLeafNode(new SimpleItem("IrisImageScanCategoryCode", AN, FieldID::TYPE_17_IPC, SubfieldIndex(0), ItemIndex(2)));
					addLeafNode(new SimpleItem("IrisImageVerticalOrientationCode", AN, FieldID::TYPE_17_IPC, SubfieldIndex(0), ItemIndex(1)));
				finishNode("IrisImageCapture", AN);

				addLeafNode(new SimpleField("IrisEyeColorAttributeCode", AN, FieldID::TYPE_17_ECL));
				addLeafNode(new SimpleField("IrisImageAcquisitionLightingSpectrumValue", AN, FieldID::TYPE_17_ALS));
				addLeafNode(new SimpleField("IrisDiameterPixelQuantity", ITL, FieldID::TYPE_17_IRD));
			finishNode("IrisImage", ITL);
		finishNode("PackageIrisImageRecord", ITL);
	}

	void RecordConverter::configureType99Converter() {
		addNode(new Converter("PackageCBEFFBiometricDataRecord", ITL));
			addLeafNode(new LiteralConverter("RecordCategoryCode", AN, "99"));
			addNode(new Converter("ImageReferenceIdentification", AN));
				addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_99_IDC, new IDCFormatter));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new Converter("CBEFFImage", AN));
				addLeafNode(new SimpleField("BinaryBase64Object", NC, FieldID::TYPE_99_BDB, new Base64Formatter));

				addNode(new Converter("ImageCaptureDetail", AN));
					addNode(new Converter("CaptureDate", AN));
						addLeafNode(new SimpleField("DateTime", NC, FieldID::TYPE_99_BCD, new UTCDateFormatter));
					finishNode("CaptureDate", AN);
					addNode(new Converter("CaptureOrganization", AN));
						addNode(new Converter("OrganizationIdentification", NC));
							addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_99_SRC));
						finishNode("OrganizationIdentification", NC);
					finishNode("CaptureOrganization", AN);
				finishNode("ImageCaptureDetail", AN);

				addNode(new SubfieldIncrementer("ImageQuality", AN));
					addNode(new Converter("QualityAlgorithmProductIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_99_BDQ, SubfieldIndex(0), ItemIndex(2)));
					finishNode("QualityAlgorithmProductIdentification", AN);
					addLeafNode(new SimpleItem("QualityValue", AN, FieldID::TYPE_99_BDQ, SubfieldIndex(0), ItemIndex(0)));
					addNode(new Converter("QualityMeasureVendorIdentification", AN));
						addLeafNode(new SimpleItem("IdentificationID", NC, FieldID::TYPE_99_BDQ, SubfieldIndex(0), ItemIndex(1)));
					finishNode("QualityMeasureVendorIdentification", AN);
				finishNode("ImageQuality", AN);

				addNode(new Converter("CBEFFFormatOwnerIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_99_BFO));
				finishNode("CBEFFFormatOwnerIdentification", AN);

				addNode(new Converter("CBEFFFormatCategoryIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_99_BFT));
				finishNode("CBEFFFormatCategoryIdentification", AN);

				addNode(new Converter("CBEFFVersionIdentification", AN));
					addLeafNode(new SimpleField("IdentificationID", NC, FieldID::TYPE_99_HDV));
				finishNode("CBEFFVersionIdentification", AN);

				addLeafNode(new SimpleField("CBEFFCategoryCode", AN, FieldID::TYPE_99_BTY));
			finishNode("CBEFFImage", AN);
		finishNode("PackageCBEFFBiometricDataRecord", ITL);
	}
}
