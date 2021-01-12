#ifndef DOCTOTEXT_EML_PARSER_H
#define DOCTOTEXT_EML_PARSER_H

#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	struct FormattingStyle;
	class Metadata;
	class Attachment;
}

using namespace doctotext;

class EMLParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		EMLParser(const std::string& file_name);
		EMLParser(const char* buffer, size_t size);
		~EMLParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		bool isEML();
		void getLinks(std::vector<Link>& links);
		void getAttachments(std::vector<Attachment>& attachments);
		std::string plainText(const FormattingStyle& formatting);
		Metadata metaData();
		bool error();
};

#endif
