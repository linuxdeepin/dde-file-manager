#ifndef DOCTOTEXT_DOC_PARSER_H
#define DOCTOTEXT_DOC_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	class FormattingStyle;
	class Metadata;
}

using namespace doctotext;

class DOCParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		DOCParser(const std::string& file_name);
		DOCParser(const char* buffer, size_t size);
		~DOCParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isDOC();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		Metadata metaData();
		bool error();
};

#endif
