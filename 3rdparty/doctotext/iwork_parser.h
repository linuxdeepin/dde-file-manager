#ifndef DOCTOTEXT_IWORK_PARSER_H
#define DOCTOTEXT_IWORK_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	class Metadata;
}

using namespace doctotext;

class IWorkParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		IWorkParser(const std::string& file_name);
		IWorkParser(const char* buffer, size_t size);
		~IWorkParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isIWork();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		bool error();
		Metadata metaData();
};

#endif
