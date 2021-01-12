#ifndef DOCTOTEXT_FORMATTING_STYLE_H
#define DOCTOTEXT_FORMATTING_STYLE_H

#include <string>

namespace doctotext
{
	enum TableStyle { TABLE_STYLE_TABLE_LOOK, TABLE_STYLE_ONE_ROW, TABLE_STYLE_ONE_COL, };
	enum UrlStyle { URL_STYLE_TEXT_ONLY, URL_STYLE_EXTENDED, URL_STYLE_UNDERSCORED, };

	class ListStyle
	{
		private:
			struct Implementation;
			Implementation* m_impl;

		public:
			ListStyle();
			~ListStyle();
			ListStyle(const ListStyle& style);
			ListStyle& operator = (const ListStyle& style);
			/*
			 * Sets leading characters for each position in the list.
			 * prefix must be a value encoded in UTF8. Default value:
			 * empty string (no prefix)
			*/
			void setPrefix(const std::string& prefix);
			void setPrefix(const char* prefix);
			const char* getPrefix() const;
	};

	struct FormattingStyle
	{
		TableStyle table_style;
		UrlStyle url_style;
		ListStyle list_style;
	};

	enum XmlParseMode { PARSE_XML, FIX_XML, STRIP_XML };
}

#endif
