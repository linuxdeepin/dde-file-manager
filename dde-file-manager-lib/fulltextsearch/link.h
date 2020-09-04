#ifndef DOCTOTEXT_LINK_H
#define DOCTOTEXT_LINK_H

#include <string>

namespace doctotext
{
	/**
		Structure of the link in parsed file.
		Example:
		Suppose that content of example.html is: "text before link <a href="target">link</a> text after link".
		As the result, example.html will contain one link. We are using URL_STYLE_TEXT_ONLY style.
		The result of the parsing this file is: "text before link link text after link".
		We should obtain one link with the following values:
		getLinkUrl() returs word "target".
		getLinkText() returns word "link".
		getLinkTextSize() returns 4 (because word "link" has four characters).
		getLinkTextPosition() returns 17 (because lenght of "text before link " is 17).
	**/
	class Link
	{
		private:
			struct Implementation;
			Implementation* impl;

		public:
			Link();
			Link(const std::string& link_url, const std::string& link_text, size_t link_text_position);
			Link(const Link& link);
			~Link();
			Link& operator = (const Link& link);
			void setLinkText(const std::string& link_text);
			void setLinkUrl(const std::string& link_url);
			void setLinkTextPosition(size_t link_text_position);
			const char* getLinkUrl() const;
			const char* getLinkText() const;
			size_t getLinkTextPosition() const;
	};
}

#endif
