#ifndef DOCTOTEXT_PDF_PARSER_H
#define DOCTOTEXT_PDF_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	class Metadata;
}

using namespace doctotext;

class PDFParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		PDFParser(const std::string& file_name);
		PDFParser(const char* buffer, size_t size);
		~PDFParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isPDF();
		void getLinks(std::vector<Link>& links);
		std::string plainText(const FormattingStyle& formatting);
		bool error();
		Metadata metaData();
};

#endif
