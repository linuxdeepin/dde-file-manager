#ifndef DOCTOTEXT_XLS_PARSER_H
#define DOCTOTEXT_XLS_PARSER_H

#include "metadata.h"
#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
}
class ThreadSafeOLEStorage;

using namespace doctotext;

class XLSParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		XLSParser(const std::string& file_name);
		XLSParser(const char* buffer, size_t size);
		~XLSParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isXLS();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		std::string plainText(ThreadSafeOLEStorage& storage, const FormattingStyle& formatting);
		Metadata metaData();
		bool error();
};

#endif
