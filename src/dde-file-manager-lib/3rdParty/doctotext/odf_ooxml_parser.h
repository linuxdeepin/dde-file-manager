#ifndef DOCTOTEXT_ODFOOXML_PARSER_H
#define DOCTOTEXT_ODFOOXML_PARSER_H

#include "common_xml_document_parser.h"

class ODFOOXMLParser : public CommonXMLDocumentParser
{
	private:
		struct ExtendedImplementation;
		ExtendedImplementation* extended_impl;
		class CommandHandlersSet;

	public:
		ODFOOXMLParser(const std::string &file_name);
		ODFOOXMLParser(const char* buffer, size_t size);
		~ODFOOXMLParser();
		bool isODFOOXML();
		std::string plainText(XmlParseMode mode, FormattingStyle& options);
		Metadata metaData();
};

#endif

 
