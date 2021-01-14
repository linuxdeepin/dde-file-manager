#ifndef DOCTOTEXT_PPT_PARSER_H
#define DOCTOTEXT_PPT_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	struct Metadata;
}

using namespace doctotext;

class PPTParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		PPTParser(const std::string& file_name);
		PPTParser(const char* buffer, size_t size);
		~PPTParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isPPT();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		Metadata metaData();
		bool error();
};

#endif
