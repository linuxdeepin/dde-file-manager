#ifndef DOCTOTEXT_XML_FIXER_H
#define DOCTOTEXT_XML_FIXER_H

#include <string>

class DocToTextXmlFixer
{
	private:
		struct Implementation;
		Implementation* Impl;

	public:
		DocToTextXmlFixer();
		~DocToTextXmlFixer();
		std::string fix(const std::string& xml) const;
};

#endif
