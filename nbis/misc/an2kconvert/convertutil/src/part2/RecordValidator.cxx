#include "part2/RecordValidator.hxx"
#include "part2/XMLElement.hxx"
#include "validate/text/TextValidator.hxx"
#include "validate/text/TableValidators.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>

namespace convert {
	namespace part2 {
		using namespace std;

		RecordValidator::~RecordValidator() {
			deleteContents<ElementValidator*, list<ElementValidator*> >(nodeStack);
		}

		RecordVariant RecordValidator::validateRecord(Record const& part2Record, ValidationLevel vl, bool ebts) {
			auto_ptr<list<RecordVariant> > recordVariants = getRecordTypeVariants(part2Record.getRecordType(), ebts);
			list<ValidationResult*> valResults;
			cout << "Processing Type " << part2Record.getRecordType() << " record......";
			for(list<RecordVariant>::const_iterator it = recordVariants->begin(); it != recordVariants->end(); it++) {
				RecordVariant recordVariant = *it;
				RecordValidator rv(part2Record.getRecordType(), recordVariant);
				valResults.push_back(rv.validate(part2Record, vl).release());
				if(!valResults.back()->hasError()) {
					if(valResults.back()->hasWarning()) {
						cout << "warning" << endl;
						valResults.back()->printWarnings(cout);
					} else {
						cout << "done" << endl;
					}
					deleteContents<ValidationResult*>(valResults);
					return recordVariant;
				}
			}

			stringstream ss;
			cout << "error" << endl;
			ss << "Type " << part2Record.getRecordType() << " record has the following errors:" << endl;
			int errorsPrinted = 0;
			for(list<ValidationResult*>::const_iterator it = valResults.begin(); it != valResults.end(); it++) {
				ValidationResult const& valResult = *(*it);
				valResult.printErrors(ss);
				errorsPrinted++;
				if(errorsPrinted != valResults.size()) {
					ss << "or" << endl;
				}
			}
			ss << endl;
			deleteContents<ValidationResult*>(valResults);
			throw ValidationError(ss.str());
		}

		RecordValidator::RecordValidator(RecordType recordType, RecordVariant recordVariant)
		: recordType(recordType), recordVariant(recordVariant) {
			switch(recordVariant) {
			case TYPE1_A: configureType1AValidator(); break;
			case TYPE1_B: configureType1BValidator(); break;
			case TYPE2_A: configureType2AValidator(); break;
			case TYPE2_B: configureType2BValidator(); break;
			case TYPE3_A: configureType3Validator(); break;
			case TYPE4_A: configureType4Validator(); break;
			case TYPE5_A: configureType5Validator(); break;
			case TYPE6_A: configureType6Validator(); break;
			case TYPE7_A: configureType7Validator(); break;
			case TYPE8_A: configureType8AValidator(); break;
			case TYPE8_B: configureType8BValidator(); break;
			case TYPE9_A: configureType9AValidator(); break;
			case TYPE9_B: configureType9BValidator(); break;
			case TYPE9_C: configureType9CValidator(); break;
			case TYPE9_D: configureType9DValidator(); break;
			case TYPE10_A: configureType10AValidator(); break;
			case TYPE10_B: configureType10BValidator(); break;
			case TYPE13_A: configureType13AValidator(); break;
			case TYPE13_B: configureType13BValidator(); break;
			case TYPE13_C: configureType13CValidator(); break;
			case TYPE14_A: configureType14AValidator(); break;
			case TYPE14_B: configureType14BValidator(); break;
			case TYPE15_A: configureType15Validator(); break;
			case TYPE16_A: configureType16Validator(); break;
			case TYPE17_A: configureType17Validator(); break;
			case TYPE99_A: configureType99Validator(); break;
			}
		}

		void RecordValidator::addNode(ElementValidator* validator) {
			nodeStack.push(validator);
		}

		void RecordValidator::finishNode(string const& elementName, XMLNamespace ns) {
			if(nodeStack.empty()) {
				throw logic_error("Part2RecordValidator.finishNode(): can't finish empty node");
				//can't finish empty node
			}
			if(ElementID(elementName, ns) != nodeStack.top()->getElementId()) {
				throw logic_error("Part2RecordValidator.finishNode(): elementId doesn't match top node");
				//elementId doesn't match top element
			}
			ElementValidator* node = nodeStack.top();
			nodeStack.pop();
			if(nodeStack.empty()) {
				rootNode = auto_ptr<ElementValidator>(node);
			} else {
				nodeStack.top()->addValidator(auto_ptr<ElementValidator>(node));
			}
		}

		void RecordValidator::addLeafNode(ElementValidator* validator) {
			if(nodeStack.empty()) {
				throw logic_error("Part2RecordValidator.addLeafNode(): can't add leaf node to empty parent");
				//can't add leaf node to empty parent
			}
			nodeStack.top()->addValidator(auto_ptr<ElementValidator>(validator));
		}

		auto_ptr<ValidationResult> RecordValidator::validate(Record const& part2Record, ValidationLevel vl) {
			if(rootNode.get() == NULL) {
				throw logic_error("Part2RecordValidator.validate(): null root node");
				//null root node
			}
			auto_ptr<ValidationResult> valResult(new ValidationResult(part2Record.getRecordType(), recordVariant));
			try {
				Context context;
				rootNode->validateElement(part2Record.getRootElement(), context, *valResult, vl);
			} catch(ValidationError&) {}
			return valResult;
		}

		void RecordValidator::configureType1AValidator() {
			addNode(new ElementValidator("PackageInformationRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(1)));

				addNode(new ElementValidator("Transaction", AN, SingleMandatory));
					addNode(new ElementValidator("TransactionDate", AN, SingleMandatory));
						addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
					finishNode("TransactionDate", AN);

					addNode(new ElementValidator("TransactionDestinationOrganization", AN, SingleMandatory));
						addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("OrganizationIdentification", NC);
						addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
					finishNode("TransactionDestinationOrganization", AN);

					addNode(new ElementValidator("TransactionOriginatingOrganization", AN, SingleMandatory));
						addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("OrganizationIdentification", NC);
						addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
					finishNode("TransactionOriginatingOrganization", AN);

					addNode(new ElementValidator("TransactionUTCDate", AN, SingleOptional));
						addLeafNode(new ElementValidator("DateTime", NC, SingleMandatory, new UTCDateValidator));
					finishNode("TransactionUTCDate", AN);

					addNode(new ElementValidator("TransactionControlIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
					finishNode("TransactionControlIdentification", AN);

					addNode(new ElementValidator("TransactionControlReferenceIdentification", AN, SingleOptional));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
					finishNode("TransactionControlReferenceIdentification", AN);

					addNode(new ElementValidator("TransactionDomain", AN, SingleOptional));
						addNode(new ElementValidator("DomainVersionNumberIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("DomainVersionNumberIdentification", AN);
						addLeafNode(new ElementValidator("OrganizationName", AN, SingleMandatory));
					finishNode("TransactionDomain", AN);

					addNode(new ElementValidator("TransactionImageResolutionDetails", AN, SingleMandatory));
						addLeafNode(new ElementValidator("NativeScanningResolutionValue", AN, SingleMandatory, new ResolutionValidator));
						addLeafNode(new ElementValidator("NominalTransmittingResolutionValue", AN, SingleMandatory, new ResolutionValidator));
					finishNode("TransactionImageResolutionDetails", AN);

					addLeafNode(new ElementValidator("TransactionMajorVersionValue", AN, SingleMandatory, new SingleValueValidator("04", RelaxedWarning)));
					addLeafNode(new ElementValidator("TransactionMinorVersionValue", AN, SingleMandatory, new SingleValueValidator("00", RelaxedWarning)));
					addLeafNode(new ElementValidator("TransactionPriorityValue", AN, SingleOptional, new NumericRangeValidator(1, 9)));
					addLeafNode(new ElementValidator("TransactionCategoryCode", AN, SingleMandatory));

					addNode(new ElementValidator("TransactionContentSummary", AN, SingleMandatory));
						addLeafNode(new ElementValidator("ContentFirstRecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(1)));
						addLeafNode(new ElementValidator("ContentRecordCount", AN, SingleMandatory, new NNIntValidator));
						addNode(new ElementValidator("ContentRecordSummary", AN, UnlimitedMandatory));
							addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
							finishNode("ImageReferenceIdentification", AN);
							addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NNIntValidator));
						finishNode("ContentRecordSummary", AN);
					finishNode("TransactionContentSummary", AN);

					addNode(new ElementValidator("TransactionCharacterSetDirectory", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("CharacterSetCommonNameCode", AN, SingleMandatory));
						addLeafNode(new ElementValidator("CharacterSetIndexCode", AN, SingleMandatory));
						addNode(new ElementValidator("CharacterSetVersionIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("CharacterSetVersionIdentification", AN);
					finishNode("TransactionCharacterSetDirectory", AN);

				finishNode("Transaction", AN);
			finishNode("PackageInformationRecord", ITL);
		}

		void RecordValidator::configureType1BValidator() {
			addNode(new ElementValidator("PackageInformationRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(1)));

				addNode(new ElementValidator("Transaction", EBTS, SingleMandatory));
					addNode(new ElementValidator("TransactionDate", AN, SingleMandatory));
						addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
					finishNode("TransactionDate", AN);

					addNode(new ElementValidator("TransactionDestinationOrganization", AN, SingleMandatory));
						addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("OrganizationIdentification", NC);
						addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
					finishNode("TransactionDestinationOrganization", AN);

					addNode(new ElementValidator("TransactionOriginatingOrganization", AN, SingleMandatory));
						addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("OrganizationIdentification", NC);
						addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
					finishNode("TransactionOriginatingOrganization", AN);

					addNode(new ElementValidator("TransactionUTCDate", AN, SingleOptional));
						addLeafNode(new ElementValidator("DateTime", NC, SingleMandatory, new UTCDateValidator));
					finishNode("TransactionUTCDate", AN);

					addNode(new ElementValidator("TransactionControlIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
					finishNode("TransactionControlIdentification", AN);

					addNode(new ElementValidator("TransactionControlReferenceIdentification", AN, SingleOptional));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
					finishNode("TransactionControlReferenceIdentification", AN);

					addNode(new ElementValidator("TransactionDomain", AN, SingleOptional));
						addNode(new ElementValidator("DomainVersionNumberIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("DomainVersionNumberIdentification", AN);
						addLeafNode(new ElementValidator("OrganizationName", AN, SingleMandatory));
					finishNode("TransactionDomain", AN);

					addNode(new ElementValidator("TransactionImageResolutionDetails", AN, SingleMandatory));
						addLeafNode(new ElementValidator("NativeScanningResolutionValue", AN, SingleMandatory, new ResolutionValidator));
						addLeafNode(new ElementValidator("NominalTransmittingResolutionValue", AN, SingleMandatory, new ResolutionValidator));
					finishNode("TransactionImageResolutionDetails", AN);

					addLeafNode(new ElementValidator("TransactionMajorVersionValue", AN, SingleMandatory, new SingleValueValidator("04", RelaxedWarning)));
					addLeafNode(new ElementValidator("TransactionMinorVersionValue", AN, SingleMandatory, new SingleValueValidator("00", RelaxedWarning)));
					addLeafNode(new ElementValidator("TransactionPriorityValue", AN, SingleOptional, new NumericRangeValidator(1, 9)));

					addNode(new ElementValidator("TransactionContentSummary", AN, SingleMandatory));
						addLeafNode(new ElementValidator("ContentFirstRecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(1)));
						addLeafNode(new ElementValidator("ContentRecordCount", AN, SingleMandatory, new NNIntValidator));
						addNode(new ElementValidator("ContentRecordSummary", AN, UnlimitedMandatory));
							addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
							finishNode("ImageReferenceIdentification", AN);
							addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NNIntValidator));
						finishNode("ContentRecordSummary", AN);
					finishNode("TransactionContentSummary", AN);

					addNode(new ElementValidator("TransactionCharacterSetDirectory", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("CharacterSetCommonNameCode", AN, SingleMandatory));
						addLeafNode(new ElementValidator("CharacterSetIndexCode", AN, SingleMandatory));
						addNode(new ElementValidator("CharacterSetVersionIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("CharacterSetVersionIdentification", AN);
					finishNode("TransactionCharacterSetDirectory", AN);

					addNode(new ElementValidator("TransactionAugmentation", EBTS, SingleMandatory));
						addLeafNode(new ElementValidator("TransactionCategoryCode", EBTS, SingleMandatory));
					finishNode("TransactionAugmentation", EBTS);
				finishNode("Transaction", EBTS);
			finishNode("PackageInformationRecord", ITL);
		}

		void RecordValidator::configureType2AValidator() {
			addNode(new ElementValidator("PackageDescriptiveTextRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(2)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);
			finishNode("PackageDescriptiveTextRecord", ITL);
		}

		void RecordValidator::configureType2BValidator() {
			addNode(new ElementValidator("PackageDescriptiveTextRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(2)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("UserDefinedDescriptiveText", ITL, SingleMandatory));
					addNode(new ElementValidator("DomainDefinedDescriptiveFields", EBTS, SingleMandatory));
						addNode(new ElementValidator("RecordForwardOrganizations", AN, ElemMin(0), ElemMax(9)));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(9, 19)));
							finishNode("OrganizationIdentification", NC);
						finishNode("RecordForwardOrganizations", AN);

						addNode(new ElementValidator("RecordTransactionData", EBTS, SingleOptional));
							addLeafNode(new ElementValidator("TransactionQueryDepthCode", EBTS, SingleOptional, new MultipleValueValidator("S|O|C")));
						finishNode("RecordTransactionData", EBTS);

						addNode(new ElementValidator("RecordActivity", EBTS, SingleOptional));
							addNode(new ElementValidator("ContributorCaseIdentificationNumber", EBTS, ElemMin(0, 1), ElemMax(5, 200)));
								addNode(new ElementValidator("ContributorCasePrefixIdentification", EBTS, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(1, 24)));
								finishNode("ContributorCasePrefixIdentification", EBTS);
								addNode(new ElementValidator("ContributorCaseIdentification", EBTS, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(1, 24)));
								finishNode("ContributorCaseIdentification", EBTS);
								addNode(new ElementValidator("ContributorCaseExtensionIdentification", EBTS, ElemMin(0, 1), ElemMax(1)));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator(2, 4)));
								finishNode("ContributorCaseExtensionIdentification", EBTS);
							finishNode("ContributorCaseIdentificationNumber", EBTS);

							addNode(new ElementValidator("FBIFileNumber", EBTS, ElemMin(0, 1), ElemMax(1)));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator(10, 10)));
							finishNode("FBIFileNumber", EBTS);

							addNode(new ElementValidator("FBILatentCaseIdentification", EBTS, ElemMin(0, 1), ElemMax(1)));
								addNode(new ElementValidator("FBILatentCaseNumber", EBTS, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new Type2LCNValidator));
								finishNode("FBILatentCaseNumber", EBTS);
								addNode(new ElementValidator("FBILatentCaseNumberExtension", EBTS, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator(4, 4)));
								finishNode("FBILatentCaseNumberExtension", EBTS);
							finishNode("FBILatentCaseIdentification", EBTS);
						finishNode("RecordActivity", EBTS);

						addNode(new ElementValidator("RecordSubject", EBTS, SingleOptional));
							addNode(new ElementValidator("PersonFBIIdentification", J, ElemMin(0), ElemMax(5, 1000)));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(9, 9)));
							finishNode("PersonFBIIdentification", J);
						finishNode("RecordSubject", EBTS);
					finishNode("DomainDefinedDescriptiveFields", EBTS);
				finishNode("UserDefinedDescriptiveText", ITL);

			finishNode("PackageDescriptiveTextRecord", ITL);
		}

		void RecordValidator::configureType3Validator() {
			addNode(new ElementValidator("PackageLowResolutionGrayscaleImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(3)));
				configureType3_6Validator();
			finishNode("PackageLowResolutionGrayscaleImageRecord", ITL);
		}

		void RecordValidator::configureType4Validator() {
			addNode(new ElementValidator("PackageHighResolutionGrayscaleImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(4)));
				configureType3_6Validator();
			finishNode("PackageHighResolutionGrayscaleImageRecord", ITL);
		}

		void RecordValidator::configureType5Validator() {
			addNode(new ElementValidator("PackageLowResolutionBinaryImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(5)));
				configureType3_6Validator();
			finishNode("PackageLowResolutionBinaryImageRecord", ITL);
		}

		void RecordValidator::configureType6Validator() {
			addNode(new ElementValidator("PackageHighResolutionBinaryImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(6)));
				configureType3_6Validator();
			finishNode("PackageHighResolutionBinaryImageRecord", ITL);
		}

		void RecordValidator::configureType3_6Validator() {
			addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
				addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new U8Validator));
			finishNode("ImageReferenceIdentification", AN);

			addNode(new ElementValidator("FingerprintImage", AN, SingleMandatory));
				addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));

				addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
					addLeafNode(new ElementValidator("CaptureResolutionCode", AN, SingleMandatory, new NumericRangeValidator(0, 1)));
				finishNode("ImageCaptureDetail", AN);

				addLeafNode(new ElementValidator("ImageCompressionAlgorithmCode", AN, SingleMandatory, new U8Validator));
				addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new U16Validator));
				addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new U16Validator));

				addNode(new ElementValidator("FingerprintImagePosition", AN, SingleMandatory));
					addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(6), new Type3_6FGPValidator));
				finishNode("FingerprintImagePosition", AN);

				addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
			finishNode("FingerprintImage", AN);
		}

		void RecordValidator::configureType7Validator() {
			addNode(new ElementValidator("PackageUserDefinedImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(7)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new U8Validator));
				finishNode("ImageReferenceIdentification", AN);
			finishNode("PackageUserDefinedImageRecord", ITL);
		}

		void RecordValidator::configureType8AValidator() {
			addNode(new ElementValidator("PackageSignatureImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(8)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new U8Validator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("SignatureImage", AN, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addLeafNode(new ElementValidator("CaptureResolutionCode", AN, SingleMandatory, new NumericRangeValidator(0, 1)));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new U16Validator));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new U16Validator));
					addLeafNode(new ElementValidator("SignatureRepresentationCode", AN, SingleMandatory, new NumericRangeValidator(0, 1)));
					addLeafNode(new ElementValidator("SignatureCategoryCode", AN, SingleMandatory, new NumericRangeValidator(0, 1)));
				finishNode("SignatureImage", AN);
			finishNode("PackageSignatureImageRecord", ITL);
		}

		void RecordValidator::configureType8BValidator() {
			addNode(new ElementValidator("PackageSignatureImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(8)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new U8Validator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("SignatureImage", AN, SingleMandatory));
					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addLeafNode(new ElementValidator("CaptureResolutionCode", AN, SingleMandatory, new NumericValueValidator(0)));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NumericValueValidator(0)));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NumericValueValidator(0)));

					addNode(new ElementValidator("SignatureImageVectorRepresentation", AN, SingleMandatory));
						addNode(new ElementValidator("SignatureImageVector", AN, ElemMin(2)));
							addLeafNode(new ElementValidator("VectorPenPressureValue", AN, SingleMandatory, new U8Validator));
							addLeafNode(new ElementValidator("VectorPositionVerticalCoordinateValue", AN, SingleMandatory, new U16Validator));
							addLeafNode(new ElementValidator("VectorPositionHorizontalCoordinateValue", AN, SingleMandatory, new U16Validator));
						finishNode("SignatureImageVector", AN);
					finishNode("SignatureImageVectorRepresentation", AN);

					addLeafNode(new ElementValidator("SignatureRepresentationCode", AN, SingleMandatory, new NumericValueValidator(2)));
					addLeafNode(new ElementValidator("SignatureCategoryCode", AN, SingleMandatory, new NumericRangeValidator(0, 1)));
				finishNode("SignatureImage", AN);
			finishNode("PackageSignatureImageRecord", ITL);
		}

		void RecordValidator::configureType9AValidator() {
			addNode(new ElementValidator("PackageMinutiaeRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(9)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addLeafNode(new ElementValidator("MinutiaeImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
				addLeafNode(new ElementValidator("MinutiaeFormatNISTStandardIndicator", AN, SingleMandatory, new SingleValueValidator("true")));

				addNode(new ElementValidator("Minutiae", ITL, SingleMandatory));
					addNode(new ElementValidator("MinutiaeNISTStandard", ITL, SingleMandatory));
						addNode(new ElementValidator("MinutiaDetail", ITL, UnlimitedOptional));
							addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
							addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
							addNode(new ElementValidator("MinutiaIdentification", AN, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
							finishNode("MinutiaIdentification", AN);
							addLeafNode(new ElementValidator("PositionThetaAngleMeasure", AN, SingleMandatory, new NNIntValidator(3, 3)));
							addLeafNode(new ElementValidator("MinutiaQualityValue", AN, SingleOptional, new NumericRangeValidator(0, 63)));
							addLeafNode(new ElementValidator("MinutiaCategoryCode", AN, SingleOptional, new Table215Validator));
							addNode(new ElementValidator("MinutiaRidgeCount", AN, UnlimitedOptional));
								addNode(new ElementValidator("RidgeCountReferenceIdentification", AN, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
								finishNode("RidgeCountReferenceIdentification", AN);
								addLeafNode(new ElementValidator("RidgeCountValue", AN, SingleMandatory, new NNIntValidator));
							finishNode("MinutiaRidgeCount", AN);
						finishNode("MinutiaDetail", ITL);
						addLeafNode(new ElementValidator("MinutiaeQuantity", AN, SingleMandatory));
						addNode(new ElementValidator("MinutiaeReadingSystem", AN, SingleOptional));
							addLeafNode(new ElementValidator("ReadingSystemCodingMethodCode", AN, SingleMandatory, new MultipleValueValidator("A|U|E|M")));
							addLeafNode(new ElementValidator("ReadingSystemName", AN, SingleMandatory));
							addNode(new ElementValidator("ReadingSystemSubsystemIdentification", AN, SingleOptional));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(2, 2)));
							finishNode("ReadingSystemSubsystemIdentification", AN);
						finishNode("MinutiaeReadingSystem", AN);
						addLeafNode(new ElementValidator("MinutiaeRidgeCountIndicator", AN, SingleMandatory, new MultipleValueValidator("false|true")));
					finishNode("MinutiaeNISTStandard", ITL);

					addNode(new ElementValidator("MinutiaeFingerCorePosition", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
						addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
					finishNode("MinutiaeFingerCorePosition", AN);

					addNode(new ElementValidator("MinutiaeFingerDeltaPosition", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
						addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator(4, 4)));
					finishNode("MinutiaeFingerDeltaPosition", AN);

					addNode(new ElementValidator("MinutiaeFingerPatternDetail", ITL, SingleMandatory));
						addLeafNode(new ElementValidator("FingerPatternCodeSourceCode", ITL, SingleMandatory, new MultipleValueValidator("T|U")));
						addLeafNode(new ElementValidator("FingerPatternCode", AN, UnlimitedMandatory));
					finishNode("MinutiaeFingerPatternDetail", ITL);
					addLeafNode(new ElementValidator("MinutiaeFingerPositionCode", AN, UnlimitedMandatory, new Table212Validator));
				finishNode("Minutiae", ITL);
			finishNode("PackageMinutiaeRecord", ITL);
		}

		void RecordValidator::configureType9BValidator() {
			addNode(new ElementValidator("PackageMinutiaeRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(9)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addLeafNode(new ElementValidator("MinutiaeImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
				addLeafNode(new ElementValidator("MinutiaeFormatNISTStandardIndicator", AN, SingleMandatory, new SingleValueValidator("true")));

				addNode(new ElementValidator("Minutiae", ITL, SingleMandatory));
					addNode(new ElementValidator("MinutiaeNISTStandard", ITL, SingleMandatory));
						addNode(new ElementValidator("MinutiaDetail", ITL, UnlimitedOptional));
							addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator(5, 5)));
							addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator(5, 5)));
							addNode(new ElementValidator("MinutiaIdentification", AN, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
							finishNode("MinutiaIdentification", AN);
							addLeafNode(new ElementValidator("PositionThetaAngleMeasure", AN, SingleMandatory, new NNIntValidator(3, 3)));
							addLeafNode(new ElementValidator("MinutiaQualityValue", AN, SingleOptional, new NumericRangeValidator(0, 63)));
							addLeafNode(new ElementValidator("MinutiaCategoryCode", AN, SingleOptional, new Table215Validator));
							addNode(new ElementValidator("MinutiaRidgeCount", AN, UnlimitedOptional));
								addNode(new ElementValidator("RidgeCountReferenceIdentification", AN, SingleMandatory));
									addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
								finishNode("RidgeCountReferenceIdentification", AN);
								addLeafNode(new ElementValidator("RidgeCountValue", AN, SingleMandatory, new NNIntValidator));
							finishNode("MinutiaRidgeCount", AN);
						finishNode("MinutiaDetail", ITL);
						addLeafNode(new ElementValidator("MinutiaeQuantity", AN, SingleMandatory));
						addNode(new ElementValidator("MinutiaeReadingSystem", AN, SingleOptional));
							addLeafNode(new ElementValidator("ReadingSystemCodingMethodCode", AN, SingleMandatory, new MultipleValueValidator("A|U|E|M")));
							addLeafNode(new ElementValidator("ReadingSystemName", AN, SingleMandatory));
							addNode(new ElementValidator("ReadingSystemSubsystemIdentification", AN, SingleOptional));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(2, 2)));
							finishNode("ReadingSystemSubsystemIdentification", AN);
						finishNode("MinutiaeReadingSystem", AN);
						addLeafNode(new ElementValidator("MinutiaeRidgeCountIndicator", AN, SingleMandatory, new MultipleValueValidator("false|true")));
					finishNode("MinutiaeNISTStandard", ITL);

					addLeafNode(new ElementValidator("MinutiaePalmPositionCode", AN, UnlimitedMandatory, new Table235Validator));
				finishNode("Minutiae", ITL);
			finishNode("PackageMinutiaeRecord", ITL);
		}

		void RecordValidator::configureType9CValidator() {
			addNode(new ElementValidator("PackageMinutiaeRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(9)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addLeafNode(new ElementValidator("MinutiaeImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
				addLeafNode(new ElementValidator("MinutiaeFormatNISTStandardIndicator", AN, SingleMandatory, new SingleValueValidator("false")));
			finishNode("PackageMinutiaeRecord", ITL);
		}

		void RecordValidator::configureType9DValidator() {
			addNode(new ElementValidator("PackageMinutiaeRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(9)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addLeafNode(new ElementValidator("MinutiaeImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
				addLeafNode(new ElementValidator("MinutiaeFormatNISTStandardIndicator", AN, SingleMandatory, new SingleValueValidator("false")));

				addNode(new ElementValidator("Minutiae", INCITS, SingleMandatory));
					addNode(new ElementValidator("CBEFFFormatOwnerIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new SingleValueValidator("27")));
					finishNode("CBEFFFormatOwnerIdentification", AN);

					addNode(new ElementValidator("CBEFFFormatCategoryIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new MultipleValueValidator("513|514")));
					finishNode("CBEFFFormatCategoryIdentification", AN);

					addNode(new ElementValidator("CBEFFProductIdentification", INCITS, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
					finishNode("CBEFFProductIdentification", INCITS);

					addNode(new ElementValidator("FingerprintImageCapture", INCITS, SingleMandatory));
						addLeafNode(new ElementValidator("CaptureDeviceCertificationText", INCITS, SingleMandatory, new MultipleValueValidator("AFFP|NONE")));
						addNode(new ElementValidator("CaptureDeviceIdentification", INCITS, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
						finishNode("CaptureDeviceIdentification", INCITS);
					finishNode("FingerprintImageCapture", INCITS);

					addNode(new ElementValidator("NISTImage", AN, SingleMandatory));
						addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NumericRangeValidator(0, 65534)));
						addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
						addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NumericRangeValidator(0, 65534)));
						addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					finishNode("NISTImage", AN);

					addLeafNode(new ElementValidator("FingerViewCode", INCITS, SingleMandatory, new NumericRangeValidator(0, 15)));
					addLeafNode(new ElementValidator("FingerPositionCode", INCITS, ElemMin(1), ElemMax(1, INF), new NumericRangeValidator(1, 10)));

					addNode(new ElementValidator("MinutiaeQuality", INCITS, ElemMin(1), ElemMax(1, INF)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
						addNode(new ElementValidator("QualityMeasureVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityMeasureVendorIdentification", AN);
					finishNode("MinutiaeQuality", INCITS);

					addLeafNode(new ElementValidator("MinutiaeQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("MinutiaDetail", ITL, UnlimitedMandatory));
						addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addNode(new ElementValidator("MinutiaIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
						finishNode("MinutiaIdentification", AN);
						addLeafNode(new ElementValidator("PositionThetaAngleMeasure", AN, SingleMandatory, new NumericRangeValidator(0, 179)));
						addLeafNode(new ElementValidator("MinutiaQualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 100)));
						addLeafNode(new ElementValidator("MinutiaCategoryCode", INCITS, SingleMandatory, new MultipleValueValidator("0|1|2")));
					finishNode("MinutiaDetail", ITL);

					addLeafNode(new ElementValidator("RidgeCountExtractionMethodCode", INCITS, SingleOptional, new MultipleValueValidator("0|1|2")));

					addNode(new ElementValidator("RidgeCountDetails", INCITS, UnlimitedOptional));
						addNode(new ElementValidator("MinutiaIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
						finishNode("MinutiaIdentification", AN);
						addNode(new ElementValidator("RidgeCountReferenceIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
						finishNode("RidgeCountReferenceIdentification", AN);
						addLeafNode(new ElementValidator("RidgeCountValue", AN, SingleMandatory, new NNIntValidator));
					finishNode("RidgeCountDetails", INCITS);

					addNode(new ElementValidator("MinutiaeFingerCorePosition", INCITS, UnlimitedOptional));
						addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PositionThetaAngleMeasure", AN, SingleMandatory, new NumericRangeValidator(0, 179)));
					finishNode("MinutiaeFingerCorePosition", INCITS);

					addNode(new ElementValidator("MinutiaeFingerDeltaPosition", INCITS, UnlimitedOptional));
						addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PositionThetaAngleMeasure", AN, SingleMandatory, new NumericRangeValidator(0, 179)));
					finishNode("MinutiaeFingerDeltaPosition", INCITS);
				finishNode("Minutiae", INCITS);
			finishNode("PackageMinutiaeRecord", ITL);
		}

		void RecordValidator::configureType10AValidator() {
			addNode(new ElementValidator("PackageFacialAndSMTImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(10)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("PhysicalFeatureImage", AN, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageColorSpaceCode", AN, SingleMandatory, new Table203Validator));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageCategoryCode", AN, SingleMandatory, new MultipleValueValidator("SCAR|MARK|TATTOO")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("PhysicalFeatureDescriptionDetail", AN, ElemMin(0), ElemMax(9, INF)));
						addNode(new ElementValidator("PhysicalFeatureColorDetail", AN, SingleOptional));
							addLeafNode(new ElementValidator("PhysicalFeaturePrimaryColorCode", AN, SingleMandatory, new Table229Validator));
							addLeafNode(new ElementValidator("PhysicalFeatureSecondaryColorCode", AN, UnlimitedOptional, new Table229Validator));
						finishNode("PhysicalFeatureColorDetail", AN);
						addLeafNode(new ElementValidator("PhysicalFeatureCategoryCode", AN, SingleMandatory, new MultipleValueValidator("SCAR|MARK|TATTOO|CHEMICAL|BRANDED|CUT")));
						addLeafNode(new ElementValidator("PhysicalFeatureClassCode", AN, SingleMandatory, new Table230Validator));
						addLeafNode(new ElementValidator("PhysicalFeatureDescriptionText", AN, SingleOptional));
						addLeafNode(new ElementValidator("PhysicalFeatureSubClassCode", AN, SingleMandatory, new Table231Validator));
					finishNode("PhysicalFeatureDescriptionDetail", AN);

					addLeafNode(new ElementValidator("PhysicalFeatureNCICCode", AN, ElemMin(1), ElemMax(3, INF)));

					addNode(new ElementValidator("PhysicalFeatureSize", AN, SingleOptional));
						addLeafNode(new ElementValidator("PhysicalFeatureHeightMeasure", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PhysicalFeatureWidthMeasure", AN, SingleMandatory, new NNIntValidator));
					finishNode("PhysicalFeatureSize", AN);
				finishNode("PhysicalFeatureImage", AN);
			finishNode("PackageFacialAndSMTImageRecord", ITL);
		}

		void RecordValidator::configureType10BValidator() {
			addNode(new ElementValidator("PackageFacialAndSMTImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(10)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("FaceImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageColorSpaceCode", AN, SingleMandatory, new Table203Validator));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageQuality", AN, ElemMin(0), ElemMax(1, 9)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
						addNode(new ElementValidator("QualityMeasureVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityMeasureVendorIdentification", AN);
					finishNode("ImageQuality", AN);

					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageCategoryCode", AN, SingleMandatory, new SingleValueValidator("FACE")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("FaceImage3DPoseAngle", AN, SingleOptional));
						addLeafNode(new ElementValidator("PosePitchAngleMeasure", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PosePitchUncertaintyValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("PoseRollAngleMeasure", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PoseRollUncertaintyValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("PoseYawAngleMeasure", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("PoseYawUncertaintyValue", AN, SingleOptional, new NNIntValidator));
					finishNode("FaceImage3DPoseAngle", AN);

					addLeafNode(new ElementValidator("FaceImageAcquisitionProfileCode", AN, UnlimitedOptional, new Table220Validator));

					addNode(new ElementValidator("FaceImageAttribute", AN, ElemMin(0), ElemMax(9, INF)));
						addLeafNode(new ElementValidator("FaceImageAttributeCode", AN, SingleMandatory, new Table221Validator));
						addLeafNode(new ElementValidator("FaceImageAttributeText", AN, SingleOptional));
					finishNode("FaceImageAttribute", AN);

					addLeafNode(new ElementValidator("FaceImageDescriptionCode", AN, ElemMin(0), ElemMax(50, INF)));
					addLeafNode(new ElementValidator("FaceImageEyeColorAttributeCode", AN, SingleOptional, new Table223Validator));

					addNode(new ElementValidator("FaceImageFeaturePoint", AN, ElemMin(0), ElemMax(88, INF)));
						addLeafNode(new ElementValidator("FeaturePointHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator(1, 4)));
						addNode(new ElementValidator("FeaturePointIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new FeaturePointCodeValidator));
						finishNode("FeaturePointIdentification", AN);
						addLeafNode(new ElementValidator("FeaturePointCategoryCode", AN, SingleMandatory, new SingleValueValidator("1")));
						addLeafNode(new ElementValidator("FeaturePointVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator(1, 4)));
					finishNode("FaceImageFeaturePoint", AN);

					addLeafNode(new ElementValidator("FaceImageHairColorAttributeCode", AN, ElemMin(0), ElemMax(2), new Table226Validator));
					addLeafNode(new ElementValidator("FaceImagePoseOffsetAngleMeasure", AN, SingleOptional, new IntRangeValidator(-180, 180)));
					addLeafNode(new ElementValidator("FaceImageSubjectPoseCode", AN, SingleOptional, new Table227Validator));

					addNode(new ElementValidator("FaceImageAcquisitionSource", ITL, SingleOptional));
						addLeafNode(new ElementValidator("CaptureSourceCode", AN, SingleMandatory, new Table228Validator));
						addLeafNode(new ElementValidator("CaptureSourceDescriptionText", ITL, SingleOptional));
					finishNode("FaceImageAcquisitionSource", ITL);
				finishNode("FaceImage", ITL);
			finishNode("PackageFacialAndSMTImageRecord", ITL);
		}

		void RecordValidator::configureType13AValidator() {
			addNode(new ElementValidator("PackageLatentImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(13)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("FingerprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new NumericRangeValidator(4, 7)));
					addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(6), new NumericRangeValidator(0, 15)));

					addNode(new ElementValidator("FingerprintImageQuality", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new Table212Validator));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageQuality", ITL);

				finishNode("FingerprintImage", ITL);
			finishNode("PackageLatentImageRecord", ITL);
		}

		void RecordValidator::configureType13BValidator() {
			addNode(new ElementValidator("PackageLatentImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(13)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("FingerprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new NumericRangeValidator(4, 7)));
					addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(6), new NumericValueValidator(19)));

					addNode(new ElementValidator("FingerprintImageMajorCasePrint", AN, SingleMandatory));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(9), new NumericRangeValidator(0, 19)));
						addLeafNode(new ElementValidator("MajorCasePrintCode", AN, ElemMin(1), ElemMax(9), new Table233Validator));
						addNode(new ElementValidator("MajorCasePrintSegmentOffset", AN, ElemMin(1), ElemMax(12)));
							addLeafNode(new ElementValidator("SegmentBottomVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentLocationCode", AN, SingleMandatory, new MultipleValueValidator("NA|PRX|DST|MED")));
							addLeafNode(new ElementValidator("SegmentFingerViewCode", AN, SingleMandatory, new MultipleValueValidator("FV1|FV2|FV3|FV4|TIP")));
							addLeafNode(new ElementValidator("SegmentLeftHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentRightHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentTopVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						finishNode("MajorCasePrintSegmentOffset", AN);
					finishNode("FingerprintImageMajorCasePrint", AN);

					addNode(new ElementValidator("FingerprintImageQuality", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new Table212Validator));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageQuality", ITL);

				finishNode("FingerprintImage", ITL);
			finishNode("PackageLatentImageRecord", ITL);
		}

		void RecordValidator::configureType13CValidator() {
			addNode(new ElementValidator("PackageLatentImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(13)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("PalmprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new NumericRangeValidator(12, 15)));
					addLeafNode(new ElementValidator("PalmPositionCode", AN, ElemMin(1), ElemMax(6), new NumericRangeValidator(20, 36)));

					addNode(new ElementValidator("PalmprintImageQuality", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("PalmPositionCode", AN, SingleMandatory, new Table235Validator));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("PalmprintImageQuality", ITL);

				finishNode("PalmprintImage", ITL);
			finishNode("PackageLatentImageRecord", ITL);
		}

		void RecordValidator::configureType14AValidator() {
			addNode(new ElementValidator("PackageFingerprintImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(14)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("FingerprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
					addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(6), new NumericRangeValidator(0, 15)));

					addNode(new ElementValidator("FingerprintImageFingerMissing", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("FingerMissingCode", ITL, SingleMandatory, new MultipleValueValidator("XX|UP")));
					finishNode("FingerprintImageFingerMissing", ITL);

					addNode(new ElementValidator("FingerprintImageSegmentPositionSquare", ITL, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("SegmentBottomVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentLeftHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentRightHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentTopVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
					finishNode("FingerprintImageSegmentPositionSquare", ITL);

					addNode(new ElementValidator("FingerprintImageNISTQuality", AN, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("NISTQualityMeasure", AN, SingleMandatory, new MultipleValueValidator("1|2|3|4|5|254|255")));
					finishNode("FingerprintImageNISTQuality", AN);

					addNode(new ElementValidator("FingerprintImageSegmentationQuality", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageSegmentationQuality", AN);

					addNode(new ElementValidator("FingerprintImageQuality", ITL, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageQuality", ITL);

					addNode(new ElementValidator("FingerprintImageSegmentPositionPolygon", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("PositionPolygonVertexQuantity", ITL, SingleMandatory, new NumericRangeValidator(3, 99)));
						addNode(new ElementValidator("PositionPolygonVertex", ITL, ElemMin(3)));
							addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						finishNode("PositionPolygonVertex", ITL);
					finishNode("FingerprintImageSegmentPositionPolygon", ITL);

				finishNode("FingerprintImage", ITL);
			finishNode("PackageFingerprintImageRecord", ITL);
		}

		void RecordValidator::configureType14BValidator() {
			addNode(new ElementValidator("PackageFingerprintImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(14)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("FingerprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
					addLeafNode(new ElementValidator("FingerPositionCode", AN, ElemMin(1), ElemMax(6), new NumericValueValidator(19)));

					addNode(new ElementValidator("FingerprintImageMajorCasePrint", AN, SingleMandatory));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(0, 10)));
						addLeafNode(new ElementValidator("MajorCasePrintCode", AN, SingleMandatory, new Table233Validator));
						addNode(new ElementValidator("MajorCasePrintSegmentOffset", AN, ElemMin(1), ElemMax(12)));
							addLeafNode(new ElementValidator("SegmentBottomVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentLocationCode", AN, SingleMandatory, new MultipleValueValidator("NA|PRX|DST|MED")));
							addLeafNode(new ElementValidator("SegmentFingerViewCode", AN, SingleMandatory, new MultipleValueValidator("FV1|FV2|FV3|FV4|TIP")));
							addLeafNode(new ElementValidator("SegmentLeftHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentRightHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("SegmentTopVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						finishNode("MajorCasePrintSegmentOffset", AN);
					finishNode("FingerprintImageMajorCasePrint", AN);

					addNode(new ElementValidator("FingerprintImageFingerMissing", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("FingerMissingCode", ITL, SingleMandatory, new MultipleValueValidator("XX|UP")));
					finishNode("FingerprintImageFingerMissing", ITL);

					addNode(new ElementValidator("FingerprintImageSegmentPositionSquare", ITL, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("SegmentBottomVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentLeftHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentRightHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						addLeafNode(new ElementValidator("SegmentTopVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
					finishNode("FingerprintImageSegmentPositionSquare", ITL);

					addNode(new ElementValidator("FingerprintImageNISTQuality", AN, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("NISTQualityMeasure", AN, SingleMandatory, new MultipleValueValidator("1|2|3|4|5|254|255")));
					finishNode("FingerprintImageNISTQuality", AN);

					addNode(new ElementValidator("FingerprintImageSegmentationQuality", AN, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageSegmentationQuality", AN);

					addNode(new ElementValidator("FingerprintImageQuality", ITL, UnlimitedOptional));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("FingerprintImageQuality", ITL);

					addNode(new ElementValidator("FingerprintImageSegmentPositionPolygon", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("FingerPositionCode", AN, SingleMandatory, new NumericRangeValidator(1, 10)));
						addLeafNode(new ElementValidator("PositionPolygonVertexQuantity", ITL, SingleMandatory, new NumericRangeValidator(3, 99)));
						addNode(new ElementValidator("PositionPolygonVertex", ITL, ElemMin(3)));
							addLeafNode(new ElementValidator("PositionHorizontalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
							addLeafNode(new ElementValidator("PositionVerticalCoordinateValue", AN, SingleMandatory, new NNIntValidator));
						finishNode("PositionPolygonVertex", ITL);
					finishNode("FingerprintImageSegmentPositionPolygon", ITL);

				finishNode("FingerprintImage", ITL);
			finishNode("PackageFingerprintImageRecord", ITL);
		}

		void RecordValidator::configureType15Validator() {
			addNode(new ElementValidator("PackagePalmprintImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(15)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("PalmprintImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("FingerprintImageImpressionCaptureCategoryCode", AN, SingleMandatory, new Table211Validator));
					addLeafNode(new ElementValidator("PalmPositionCode", AN, SingleMandatory, new Table235Validator));

					addNode(new ElementValidator("PalmprintImageQuality", ITL, ElemMin(0), ElemMax(4)));
						addLeafNode(new ElementValidator("PalmPositionCode", AN, SingleMandatory, new Table235Validator));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addNode(new ElementValidator("QualityAlgorithmVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityAlgorithmVendorIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
					finishNode("PalmprintImageQuality", ITL);

				finishNode("PalmprintImage", ITL);
			finishNode("PackagePalmprintImageRecord", ITL);
		}

		void RecordValidator::configureType16Validator() {
			addNode(new ElementValidator("PackageUserDefinedTestingImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(16)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("TestImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("BinaryDescriptionText", NC, SingleMandatory));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageColorSpaceCode", AN, SingleMandatory, new Table203Validator));
					addLeafNode(new ElementValidator("ImageCommentText", AN, SingleOptional));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table1Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageQuality", AN, SingleOptional));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
						addNode(new ElementValidator("QualityMeasureVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityMeasureVendorIdentification", AN);
					finishNode("ImageQuality", AN);

					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
				finishNode("TestImage", ITL);
			finishNode("PackageUserDefinedTestingImageRecord", ITL);
		}

		void RecordValidator::configureType17Validator() {
			addNode(new ElementValidator("PackageIrisImageRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(17)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("IrisImage", ITL, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));
					addLeafNode(new ElementValidator("ImageBitsPerPixelQuantity", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("Date", NC, SingleMandatory, new DateValidator));
						finishNode("CaptureDate", AN);
						addLeafNode(new ElementValidator("CaptureHorizontalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
						addLeafNode(new ElementValidator("CaptureVerticalPixelDensityValue", AN, SingleOptional, new NNIntValidator));
						addLeafNode(new ElementValidator("CaptureDeviceMonitoringModeCode", AN, SingleOptional, new Table219Validator));
					finishNode("ImageCaptureDetail", AN);

					addLeafNode(new ElementValidator("ImageColorSpaceCode", AN, SingleMandatory, new Table203Validator));
					addLeafNode(new ElementValidator("ImageCommentText", AN, ElemMin(0), ElemMax(1, INF)));
					addLeafNode(new ElementValidator("ImageCompressionAlgorithmText", AN, SingleMandatory, new Table201Validator));
					addLeafNode(new ElementValidator("ImageHorizontalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageHorizontalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));

					addNode(new ElementValidator("ImageQuality", AN, ElemMin(0), ElemMax(1, INF)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
						addNode(new ElementValidator("QualityMeasureVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityMeasureVendorIdentification", AN);
					finishNode("ImageQuality", AN);

					addLeafNode(new ElementValidator("ImageScaleUnitsCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("ImageVerticalLineLengthPixelQuantity", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("ImageVerticalPixelDensityValue", AN, SingleMandatory, new NNIntValidator));
					addLeafNode(new ElementValidator("IrisEyePositionCode", AN, SingleMandatory, new MultipleValueValidator("0|1|2")));
					addLeafNode(new ElementValidator("IrisEyeRotationAngleMeasure", AN, SingleOptional, new HexValidator(4, 4)));
					addLeafNode(new ElementValidator("IrisEyeRotationUncertaintyValueText", AN, SingleOptional, new HexValidator(4, 4)));

					addNode(new ElementValidator("IrisImageCapture", AN, ElemMin(0, 1), ElemMax(1)));
						addNode(new ElementValidator("CaptureDeviceGlobalIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new TextLengthValidator(16, 16)));
						finishNode("CaptureDeviceGlobalIdentification", AN);
						addNode(new ElementValidator("CaptureDeviceIdentification", AN, SingleOptional));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new Type17DUIValidator));
						finishNode("CaptureDeviceIdentification", AN);
						addLeafNode(new ElementValidator("CaptureDeviceMakeText", AN, SingleOptional));
						addLeafNode(new ElementValidator("CaptureDeviceModelText", AN, SingleOptional));
						addLeafNode(new ElementValidator("CaptureDeviceSerialNumberText", AN, SingleOptional));
						addLeafNode(new ElementValidator("IrisImageHorizontalOrientationCode", AN, SingleOptional, new MultipleValueValidator("0|1|2")));
						addLeafNode(new ElementValidator("IrisImageScanCategoryCode", AN, SingleOptional, new MultipleValueValidator("0|1|2|3")));
						addLeafNode(new ElementValidator("IrisImageVerticalOrientationCode", AN, SingleOptional, new MultipleValueValidator("0|1|2")));
					finishNode("IrisImageCapture", AN);

					addLeafNode(new ElementValidator("IrisEyeColorAttributeCode", AN, SingleOptional, new Table223Validator));
					addLeafNode(new ElementValidator("IrisImageAcquisitionLightingSpectrumValue", AN, SingleOptional, new MultipleValueValidator("NIR|VIS|OTHER")));
					addLeafNode(new ElementValidator("IrisDiameterPixelQuantity", ITL, SingleOptional, new NNIntValidator));
				finishNode("IrisImage", ITL);
			finishNode("PackageIrisImageRecord", ITL);
		}

		void RecordValidator::configureType99Validator() {
			addNode(new ElementValidator("PackageCBEFFBiometricDataRecord", ITL, SingleMandatory));
				addLeafNode(new ElementValidator("RecordCategoryCode", AN, SingleMandatory, new NumericValueValidator(99)));
				addNode(new ElementValidator("ImageReferenceIdentification", AN, SingleMandatory));
					addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator));
				finishNode("ImageReferenceIdentification", AN);

				addNode(new ElementValidator("CBEFFImage", AN, SingleMandatory));
					addLeafNode(new ElementValidator("BinaryBase64Object", NC, SingleMandatory, new Base64Validator));

					addNode(new ElementValidator("ImageCaptureDetail", AN, SingleMandatory));
						addNode(new ElementValidator("CaptureDate", AN, SingleMandatory));
							addLeafNode(new ElementValidator("DateTime", NC, SingleMandatory, new UTCDateValidator));
						finishNode("CaptureDate", AN);
						addNode(new ElementValidator("CaptureOrganization", AN, SingleMandatory));
							addNode(new ElementValidator("OrganizationIdentification", NC, SingleMandatory));
								addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory));
							finishNode("OrganizationIdentification", NC);
							addLeafNode(new ElementValidator("OrganizationName", NC, SingleOptional));
						finishNode("CaptureOrganization", AN);
					finishNode("ImageCaptureDetail", AN);

					addNode(new ElementValidator("ImageQuality", AN, ElemMin(0), ElemMax(1, INF)));
						addNode(new ElementValidator("QualityAlgorithmProductIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NumericRangeValidator(1, 65535)));
						finishNode("QualityAlgorithmProductIdentification", AN);
						addLeafNode(new ElementValidator("QualityValue", AN, SingleMandatory, new NumericRangeValidator(0, 255)));
						addNode(new ElementValidator("QualityMeasureVendorIdentification", AN, SingleMandatory));
							addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
						finishNode("QualityMeasureVendorIdentification", AN);
					finishNode("ImageQuality", AN);

					addNode(new ElementValidator("CBEFFFormatOwnerIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
					finishNode("CBEFFFormatOwnerIdentification", AN);

					addNode(new ElementValidator("CBEFFFormatCategoryIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new HexValidator(4, 4)));
					finishNode("CBEFFFormatCategoryIdentification", AN);

					addNode(new ElementValidator("CBEFFVersionIdentification", AN, SingleMandatory));
						addLeafNode(new ElementValidator("IdentificationID", NC, SingleMandatory, new NNIntValidator(4, 4)));
					finishNode("CBEFFVersionIdentification", AN);

					addLeafNode(new ElementValidator("CBEFFCategoryCode", AN, SingleMandatory, new Table240Validator));
				finishNode("CBEFFImage", AN);
			finishNode("PackageCBEFFBiometricDataRecord", ITL);
		}
	}
}
