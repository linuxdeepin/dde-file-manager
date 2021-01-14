#ifndef DOCTOTEXT_TXT_PARSER_H
#define DOCTOTEXT_TXT_PARSER_H

#include <string>

class TXTParser
{
	private:
		struct Implementation;
		Implementation* impl;

	public:
		TXTParser(const std::string& file_name);
		TXTParser(const char* buffer, size_t size);
		~TXTParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		std::string plainText();
		bool error();
};

#endif
