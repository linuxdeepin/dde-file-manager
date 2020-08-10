#ifndef DOCTOTEXT_XLSB_PARSER_H
#define DOCTOTEXT_XLSB_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	class Metadata;
}

using namespace doctotext;

class XLSBParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		XLSBParser(const std::string& file_name);
		XLSBParser(const char* buffer, size_t size);
		~XLSBParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isXLSB();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		bool error();
		Metadata metaData();
};

#endif
