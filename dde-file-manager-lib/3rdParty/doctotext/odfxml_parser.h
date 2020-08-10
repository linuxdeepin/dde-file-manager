#ifndef DOCTOTEXT_ODFXML_PARSER_H
#define DOCTOTEXT_ODFXML_PARSER_H

#include "common_xml_document_parser.h"

class ODFXMLParser : public CommonXMLDocumentParser
{
	private:
		struct ExtendedImplementation;
		ExtendedImplementation* extended_impl;
		class CommandHandlersSet;

	public:
		ODFXMLParser(const std::string &file_name);
		ODFXMLParser(const char* buffer, size_t size);
		~ODFXMLParser();
		bool isODFXML();
		std::string plainText(XmlParseMode mode, FormattingStyle& formatting_style);
		Metadata metaData();
};

#endif


