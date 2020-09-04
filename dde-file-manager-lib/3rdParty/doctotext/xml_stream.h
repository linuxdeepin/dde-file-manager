#ifndef DOCTOTEXT_XML_STREAM_H
#define DOCTOTEXT_XML_STREAM_H

#include <string>

class XmlStream
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		XmlStream(const std::string& xml, bool manage_xml_parser, int xml_parse_options = 0);
		~XmlStream();
		operator bool();
		void next();
		void levelDown();
		void levelUp();
		char* content();
		std::string name();
		std::string fullName();
		std::string stringValue();
		std::string attribute(const std::string& attr_name);
};

#endif


