#ifndef DOCTOTEXT_RTF_PARSER_H
#define DOCTOTEXT_RTF_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	class Metadata;
}
using namespace doctotext;

class RTFParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		RTFParser(const std::string& file_name);
		RTFParser(const char* buffer, size_t size);
		~RTFParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isRTF();
		void getLinks(std::vector<Link>& links);
		std::string plainText();
		Metadata metaData();
		bool error();
};

#endif
