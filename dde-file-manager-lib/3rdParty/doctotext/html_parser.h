#ifndef DOCTOTEXT_HTML_PARSER_H
#define DOCTOTEXT_HTML_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	class Metadata;
}

using namespace doctotext;

class HTMLParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		HTMLParser(const std::string& file_name);
		HTMLParser(const char* buffer, size_t size);
		~HTMLParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isHTML();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		Metadata metaData();
		///turns off charset decoding. It may be useful, if we want to decode data ourself (EML parser is an example).
		void skipCharsetDecoding();
		bool error();
};

#endif
