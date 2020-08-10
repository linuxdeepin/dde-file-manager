/****************************************************************************
**
** DocToText - Converts DOC, XLS, XLSB, PPT, RTF, ODF (ODT, ODS, ODP),
**             OOXML (DOCX, XLSX, PPTX), iWork (PAGES, NUMBERS, KEYNOTE),
**             ODFXML (FODP, FODS, FODT), PDF, EML and HTML documents to plain text.
**             Extracts metadata and annotations.
**
** Copyright (c) 2006-2013, SILVERCODERS(R)
** http://silvercoders.com
**
** Project homepage: http://silvercoders.com/en/products/doctotext
**
** This program may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING.GPL included in the
** packaging of this file.
**
** Please remember that any attempt to workaround the GNU General Public
** License using wrappers, pipes, client/server protocols, and so on
** is considered as license violation. If your program, published on license
** other than GNU General Public License version 2, calls some part of this
** code directly or indirectly, you have to buy commercial license.
** If you do not like our point of view, simply do not use the product.
**
** Licensees holding valid commercial license for this product
** may use this file in accordance with the license published by
** SILVERCODERS and appearing in the file COPYING.COM
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
*****************************************************************************/

#include "html_parser.h"

#include <algorithm>
#include "data_stream.h"
#include <fstream>
#include "entities.h"
#include "htmlcxx/html/Node.h"
#include "htmlcxx/html/ParserSax.h"
#include "htmlcxx/html/CharsetConverter.h"
#include <list>
#include "metadata.h"
#include "misc.h"
#include <iostream>
#include "charsetdetect.h"

using namespace htmlcxx::HTML;

class DocToTextSaxParser : public ParserSax
{
	private:
		std::vector<Link>& m_links;
		std::string& m_text;
		std::string& m_html_content;
		const FormattingStyle& m_formatting;
		bool m_in_title;
		bool m_in_style;
		bool m_first_in_seg; // do not insert paragraph breaks if we are in the beginning of document, paragraph or table cell
		bool m_in_link;
		bool m_in_script;
		bool m_in_list;
		bool m_in_table;
		bool m_turn_off_ul_enumeration;
		bool m_turn_off_ol_enumeration;
		std::string m_style_text;
		std::string m_curr_link_url;
		std::string m_curr_link_text;
		std::string* m_curr_text_block;
		std::string m_charset;
		std::ostream& m_log_stream;
		bool m_verbose_logging;
		htmlcxx::CharsetConverter* m_converter;
		char* m_decoded_buffer;	//for decoding html entities
		size_t m_decoded_buffer_size;
		bool m_skip_decoding;

		struct HtmlContainer
		{
			std::string m_curr_text;

			virtual ~HtmlContainer(){}
			virtual void endRow(){}
			virtual void endLine(){}
			virtual void endCol(){}

			virtual std::vector<svector>* getRows()
			{
				return NULL;
			}

			virtual std::vector<std::string>* getLines()
			{
				return NULL;
			}

			virtual bool isList()
			{
				return false;
			}

			virtual bool isTable()
			{
				return false;
			}

			//methods only for lists:
			virtual bool isVisibleListEnumeration()
			{
				return false;
			}

			virtual void showListEnumeration(bool show){}
		};

		struct HtmlTable : public HtmlContainer
		{
			svector m_curr_row_cells;
			std::vector<svector> m_rows;

			bool isTable()
			{
				return true;
			}

			std::vector<svector>* getRows()
			{
				return &m_rows;
			}

			void endCol()
			{
				m_curr_row_cells.push_back(m_curr_text);
				m_curr_text.clear();
			}

			void endRow()
			{
				m_rows.push_back(m_curr_row_cells);
				m_curr_row_cells.clear();
			}
		};

		struct HtmlList : public HtmlContainer
		{
			std::vector<std::string> m_lines;
			bool m_show_list_enumeration;

			HtmlList() : m_show_list_enumeration(true){}

			bool isList()
			{
				return true;
			}

			std::vector<std::string>* getLines()
			{
				return &m_lines;
			}

			void endLine()
			{
				m_lines.push_back(m_curr_text);
				m_curr_text.clear();
			}

			bool isVisibleListEnumeration()
			{
				return m_show_list_enumeration;
			}

			void showListEnumeration(bool show)
			{
				m_show_list_enumeration = show;
			}
		};

		std::list<HtmlContainer*> m_html_containers;
		HtmlContainer* m_curr_html_container;

		void addTextToCurrBlock(const std::string& text)
		{
			*m_curr_text_block += text;
			m_first_in_seg = false;
		}

		bool lastIsSpaceInCurrBlock()
		{
			return m_curr_text_block->length() > 0 && isspace(*m_curr_text_block->rbegin());
		}

		std::string normalizeSpaces(const std::string& s, bool prev_is_space)
		{
			std::string normalized;
			bool prev_space = prev_is_space;
			for (std::string::const_iterator i = s.begin(); i != s.end(); i++)
				if (isspace(*i))
				{
					if (!prev_space)
					{
						normalized += ' ';
						prev_space = true;
					}
				}
				else
				{
					normalized += *i;
					prev_space = false;
				}
			return normalized;
		}

		void convertToUtf8(std::string& text)
		{
			if (!m_skip_decoding)
			{
				if (m_converter)
					text = m_converter->convert(text);
			}
			#warning TODO: Check if libxml2 provides such a functionality. Similar function in html library does not work for some entities
			if (m_decoded_buffer_size < text.length() * 2)
			{
				if (m_decoded_buffer)
					delete[] m_decoded_buffer;
				m_decoded_buffer = NULL;
			}
			if (!m_decoded_buffer)
			{
				m_decoded_buffer = new char[text.length() * 2];
				m_decoded_buffer_size = text.length() * 2;
			}
			size_t size = decode_html_entities_utf8(m_decoded_buffer, text.data());
			text = std::string(m_decoded_buffer, size);
		}

		void createCharsetConverter()
		{
			if (!m_skip_decoding && m_converter == NULL && m_charset != "utf-8" && m_charset != "UTF-8")
			{
				try
				{
					m_converter = new htmlcxx::CharsetConverter(m_charset, "UTF-8");
				}
				catch (htmlcxx::CharsetConverter::Exception& ex)
				{
					m_log_stream << "Warning: Cant convert text to UTF-8 from " + m_charset;
					if (m_converter)
						delete m_converter;
					m_converter = NULL;
				}
			}
		}

		void parseStyleAttributes(const std::string& style, std::map<std::string, std::string>& attributes)
		{
			#warning TODO: I have not found similar function in htmlcxx, but my searches were perfunctory
			bool reading_name = true, reading_value = false;
			std::string name, value;
			for (size_t i = 0; i < style.length(); ++i)
			{
				if (isspace(style[i]))
					continue;
				if (reading_name)
				{
					if (style[i] == ':')
					{
						reading_name = false;
						reading_value = true;
						continue;
					}
					name += style[i];
				}
				else if (reading_value)
				{
					if (style[i] == ';')
					{
						reading_name = true;
						reading_value = false;
						attributes[name] = value;
						continue;
					}
					value += style[i];
				}
			}
		}

		void parseCSS()
		{
			#warning TODO: CSS selector in htmlcxx is definitely not thread-safe. For now, we only need to know\
			about one attribute (list-style). This function only fixes the problem with automatic HTML files\
			produced by LibreOffice/OpenOffice. Those programs use <ol> and <ul>, but usually turn off\
			automatic list enumeration (list-style: none), and use own enumeration. So we have a small problem.\
			We can obtain ouput like this: "1. 1. first", "2. 2. second" etc. We should repair htmlcxx or find something\
			better if we encounter a bigger problem in the future. This function is really primitive.

			//we should look for something like: ol/ul/li { list-style: none }
			size_t search_position = 0;
			size_t found_position = 0;
			while ((found_position = m_style_text.find("list-style:", search_position)) != std::string::npos)
			{
				search_position = found_position + 11;
				//first, obtain style name:
				std::string style_name;
				for (size_t i = found_position + 11; i < m_style_text.length(); ++i, ++search_position)
				{
					if (m_style_text[i] == ';')
						break;
					if (!isspace(m_style_text[i]))
						style_name += m_style_text[i];
				}
				//now obtain which tags this rule (list-style) concerns.
				if (style_name == "none")
				{
					std::string match;
					int match_index;
					for (size_t i = found_position; i > 0; --i)
					{
						if (m_style_text[i] == '{')
						{
							match_index = i - 1;
							break;
						}
					}
					while (match_index >= 0)
					{
						if (isspace(m_style_text[match_index]))
						{
							if (match.length() > 0)
								break;
						}
						else
							match = m_style_text[match_index] + match;
						--match_index;
					}
					if (match == "li")
					{
						m_turn_off_ol_enumeration = true;
						m_turn_off_ul_enumeration = true;
					}
					else if (match == "ul")
						m_turn_off_ul_enumeration = true;
					else if (match == "ol")
						m_turn_off_ol_enumeration = true;
				}
			}
		}

	protected:
		virtual void foundTag(Node node, bool isEnd)
		{
			if (isEnd)
			{
				if (strcasecmp(node.tagName().c_str(), "title") == 0)
					m_in_title = false;
				else if (strcasecmp(node.tagName().c_str(), "style") == 0)
				{
					m_in_style = false;
					parseCSS();
				}
				else if (!m_in_list && strcasecmp(node.tagName().c_str(), "div") == 0)
				{
					if (m_text.length() > 0 && m_text[m_text.length() - 1] != '\n')
						addTextToCurrBlock("\n");
				}
				else if (strcasecmp(node.tagName().c_str(), "p") == 0)
				{
					if (!m_first_in_seg)
					{
						if (!m_in_list)
							addTextToCurrBlock("\n");
						m_first_in_seg = true;
					}
				}
				else if (strcasecmp(node.tagName().c_str(), "a") == 0 && m_in_link)
				{
					if (m_in_link)
					{
						m_in_link = false;
						if (m_curr_html_container)
							m_curr_text_block = &m_curr_html_container->m_curr_text;
						else
							m_curr_text_block = &m_text;
						if (m_curr_link_url.length() > 0)
						{
							std::string link_text = formatUrl(m_curr_link_url, m_curr_link_text, m_formatting);
							convertToUtf8(link_text);
							Link link(m_curr_link_url, link_text, 0);
							m_links.push_back(link);
							std::string text;
							insertSpecialLinkBlockIntoText(text, link);
							addTextToCurrBlock(text);
						}
						else
						{
							std::string url = formatUrl(m_curr_link_url, m_curr_link_text, m_formatting);
							addTextToCurrBlock(url);
						}
						addTextToCurrBlock(" ");
					}
				}
				else if (strcasecmp(node.tagName().c_str(), "table") == 0 && m_in_table)
				{
					m_html_containers.pop_back();
					if (m_html_containers.size() > 0)
					{
						m_in_list = m_html_containers.back()->isList();
						m_in_table = m_html_containers.back()->isTable();
						m_curr_text_block = &m_html_containers.back()->m_curr_text;
						addTextToCurrBlock(formatTable(*m_curr_html_container->getRows(), m_formatting));
						delete m_curr_html_container;
						m_curr_html_container = m_html_containers.back();
					}
					else
					{
						m_in_table = false;
						m_curr_text_block = &m_text;
						addTextToCurrBlock(formatTable(*m_curr_html_container->getRows(), m_formatting));
						delete m_curr_html_container;
						m_curr_html_container = NULL;
					}
				}
				else if (strcasecmp(node.tagName().c_str(), "tr") == 0 && m_in_table)
				{
					m_curr_html_container->endRow();
				}
				else if (strcasecmp(node.tagName().c_str(), "span") == 0 ||
						 strcasecmp(node.tagName().c_str(), "button") == 0)
				{
					addTextToCurrBlock(" ");
				}
				else if ((strcasecmp(node.tagName().c_str(), "td") == 0 || strcasecmp(node.tagName().c_str(), "th") == 0) && m_in_table)
				{
					m_curr_html_container->endCol();
				}
				else if ((strcasecmp(node.tagName().c_str(), "script") == 0 || strcasecmp(node.tagName().c_str(), "iframe") == 0) && m_in_script)
				{
					m_in_script = false;
				}
				else if (m_in_list && (strcasecmp(node.tagName().c_str(), "ul") == 0
						 || strcasecmp(node.tagName().c_str(), "ol") == 0))
				{
					bool is_ordered = node.tagName() == "ol";
					m_html_containers.pop_back();
					if (m_html_containers.size() > 0)
					{
						m_in_list = m_html_containers.back()->isList();
						m_in_table = m_html_containers.back()->isTable();
						m_curr_text_block = &m_html_containers.back()->m_curr_text;
					}
					else
					{
						m_in_list = false;
						m_curr_text_block = &m_text;
					}
					if (m_curr_html_container->isVisibleListEnumeration())
					{
						if (!is_ordered)
							addTextToCurrBlock(formatList(*m_curr_html_container->getLines(), m_formatting));
						else
							addTextToCurrBlock(formatNumberedList(*m_curr_html_container->getLines()));
					}
					else
					{
						FormattingStyle formatting;
						formatting.list_style.setPrefix("");
						addTextToCurrBlock(formatList(*m_curr_html_container->getLines(), formatting));
					}
					delete m_curr_html_container;
					m_curr_html_container = NULL;
					if (m_html_containers.size() > 0)
						m_curr_html_container = m_html_containers.back();
					addTextToCurrBlock("\n");
				}
				else if (strcasecmp(node.tagName().c_str(), "br") == 0)
				{
					addTextToCurrBlock("\n");
				}
				else if (m_in_list && (strcasecmp(node.tagName().c_str(), "li") == 0))
				{
					m_curr_html_container->endLine();
				}
			}
			else if (strcasecmp(node.tagName().c_str(), "title") == 0)
				m_in_title = true;
			else if (strcasecmp(node.tagName().c_str(), "style") == 0)
				m_in_style = true;
			else if (strcasecmp(node.tagName().c_str(), "p") == 0)
			{
				if (!m_first_in_seg)
				{
					if (!m_in_list)
						addTextToCurrBlock("\n");
					m_first_in_seg = true;
				}
			}
			else if (strcasecmp(node.tagName().c_str(), "ol") == 0
					 || strcasecmp(node.tagName().c_str(), "ul") == 0)
			{
				node.parseAttributes();
				addTextToCurrBlock("\n");
				m_in_list = true;
				m_in_table = false;
				m_curr_html_container = new HtmlList;
				std::pair<bool, std::string> style = node.attribute("style");
				std::map<std::string, std::string> style_attributes;
				if (style.first)
					parseStyleAttributes(style.second, style_attributes);
				if (style_attributes.find("list-style") == style_attributes.end())
				{
					if (node.tagName() == "ol" && m_turn_off_ol_enumeration)
						m_curr_html_container->showListEnumeration(false);
					else if (node.tagName() == "ul" && m_turn_off_ul_enumeration)
						m_curr_html_container->showListEnumeration(false);
				}
				else
				{
					if (style_attributes["list-style"] == "none")
						m_curr_html_container->showListEnumeration(false);
				}
				m_html_containers.push_back(m_curr_html_container);
				m_curr_text_block = &m_curr_html_container->m_curr_text;
			}
			else if (strcasecmp(node.tagName().c_str(), "br") == 0)
			{
				addTextToCurrBlock("\n");
			}
			else if (strcasecmp(node.tagName().c_str(), "script") == 0 || strcasecmp(node.tagName().c_str(), "iframe") == 0)
			{
				m_in_script = true;
			}
			else if (strcasecmp(node.tagName().c_str(), "a") == 0)
			{
				node.parseAttributes();
				std::pair<bool, std::string> href = node.attribute("href");
				if (href.first && href.second.length() > 0)
				{
					m_in_link = true;
					m_curr_link_url = href.second;
					//skip target if begins with "#" or "javascript"
					if (m_curr_link_url[0] == '#' || m_curr_link_url.find("javascript") == 0)
						m_curr_link_url = "";
					else
						convertToUtf8(m_curr_link_url);
					m_curr_link_text = "";
					m_curr_text_block = &m_curr_link_text;
				}
			}
			else if (strcasecmp(node.tagName().c_str(), "img") == 0)
			{
				node.parseAttributes();
				std::pair<bool, std::string> title = node.attribute("alt");
				if (title.first)
				{
					convertToUtf8(title.second);
					addTextToCurrBlock(title.second + "\n");
				}
			}
			else if (strcasecmp(node.tagName().c_str(), "table") == 0)
			{
				m_in_list = false;
				m_in_table = true;
				m_curr_html_container = new HtmlTable;
				m_html_containers.push_back(m_curr_html_container);
				m_curr_text_block = &m_curr_html_container->m_curr_text;
			}
			else if (strcasecmp(node.tagName().c_str(), "body") == 0)
			{
				//if we still dont have an ancoding, we can try guess it.
				if (m_charset.empty())
				{
					csd_t charset_detector = csd_open();
					if (charset_detector == (csd_t)-1)
					{
						m_log_stream << "Warning: Could not create charset detector\n";
					}
					else
					{
						csd_consider(charset_detector, m_html_content.c_str(), m_html_content.length());
						const char* res = csd_close(charset_detector);
						if (res != NULL)
						{
							m_charset = std::string(res);
							if (m_verbose_logging)
								m_log_stream << "Could not found explicit information about encoding. Estimated encoding: " + m_charset + "\n";
							createCharsetConverter();
						}
					}
					//if we still dont know which encoding is used...
					if (m_charset.empty())
					{
						if (m_verbose_logging)
							m_log_stream << "Could not detect encoding. Document is assumed to be encoded in UTF-8\n";
						m_charset = "UTF-8";
					}
				}
			}
			else if (strcasecmp(node.tagName().c_str(), "meta") == 0)
			{
				//meta can contain information about encoding. If we still dont have information about it,
				//maybe this tag will show us encoding for this document.
				if (m_charset.empty())
				{
					node.parseAttributes();
					//it can be something like: <meta content="text/html; charset=utf-8">...
					std::pair<bool, std::string> content = node.attribute("content");
					if (content.first)
					{
						size_t charset_pos = content.second.find("charset");
						if (charset_pos != std::string::npos)
						{
							charset_pos += 7;
							while (charset_pos < content.second.length())
							{
								if (content.second[charset_pos] == ';')
									break;
								if (!isspace(content.second[charset_pos]) && content.second[charset_pos] != '=')
									m_charset += content.second[charset_pos];
								++charset_pos;
							}
						}
					}
					else
					{
						//or maybe it is <meta charset="encoding">...
						std::pair<bool, std::string> charset = node.attribute("charset");
						if (charset.first)
						{
							m_charset = charset.second;
						}
					}
					if (!m_charset.empty())
					{
						if (m_verbose_logging)
							m_log_stream << "Following encoding was detected: " + m_charset + "\n";
						createCharsetConverter();
					}
				}
			}
		}

		virtual void foundText(Node node)
		{
			if (m_in_style)
			{
				m_style_text += node.text();
				return;
			}
			if (m_in_title || m_in_script)
				return;
			std::string text = normalizeSpaces(node.text(), lastIsSpaceInCurrBlock());
			convertToUtf8(text);
			addTextToCurrBlock(text);
		}

		virtual void endParsing()
		{
			decodeSpecialLinkBlocks(m_text, m_links, m_log_stream);
			if (m_decoded_buffer)
				delete[] m_decoded_buffer;
			if (m_converter)
				delete m_converter;
		}

		virtual void beginParsing()
		{
			//if this is xhtml document, information about encoding may be stored between <?xml and ?>.
			//htmlcxx seems not to parse this fragment, so I should do this manually
			size_t initial_xml_start_pos = m_html_content.find("<?xml");
			size_t initial_xml_end_pos = m_html_content.find("?>");
			if (initial_xml_start_pos != std::string::npos && initial_xml_end_pos != std::string::npos
					&& initial_xml_end_pos > initial_xml_start_pos && initial_xml_end_pos < m_html_content.length())
			{
				std::string initial_xml = m_html_content.substr(initial_xml_start_pos, initial_xml_end_pos - initial_xml_start_pos);
				std::transform(initial_xml.begin(), initial_xml.end(), initial_xml.begin(), ::tolower);

				size_t encoding_pos = initial_xml.find("encoding");
				if (encoding_pos != std::string::npos)
				{
					encoding_pos += 7;
					while (encoding_pos < initial_xml.length() && initial_xml[encoding_pos] != '\"')
						++encoding_pos;
					++encoding_pos;
					while (encoding_pos < initial_xml.length() && initial_xml[encoding_pos] != '\"')
					{
						m_charset += initial_xml[encoding_pos];
						++encoding_pos;
					}
					if (!m_charset.empty())
					{
						if (m_verbose_logging)
							m_log_stream << "Following encoding was detected: " + m_charset + "\n";
						createCharsetConverter();
					}
				}
			}
		}

	public:
		DocToTextSaxParser(std::string& html_content, std::string& text, const FormattingStyle& formatting, std::ostream& log_stream, std::vector<Link>& links, bool verbose_logging, bool skip_decoding)
			: m_text(text), m_formatting(formatting), m_in_title(false), m_in_style(false), m_in_link(false), m_converter(NULL), m_decoded_buffer(NULL),
			  m_first_in_seg(false), m_in_script(false), m_in_list(false), m_log_stream(log_stream), m_links(links), m_decoded_buffer_size(0),
			  m_curr_html_container(NULL), m_curr_text_block(&m_text), m_in_table(false), m_turn_off_ul_enumeration(false), m_turn_off_ol_enumeration(false),
			  m_html_content(html_content), m_verbose_logging(verbose_logging), m_skip_decoding(skip_decoding)
		{
		}

		~DocToTextSaxParser()
		{
			std::list<HtmlContainer*>::iterator it = m_html_containers.begin();
			while (it != m_html_containers.end())
			{
				delete *it;
				++it;
			}
			m_html_containers.clear();
		}
};

class DocToTextMetaSaxParser : public ParserSax
{
	private:
		Metadata& m_meta;
		std::ostream& m_log_stream;

	protected:
		virtual void foundTag(Node node, bool isEnd)
		{
			if (strcasecmp(node.tagName().c_str(), "meta") == 0)
			{
				node.parseAttributes();
				std::pair<bool, std::string> name = node.attribute("name");
				std::pair<bool, std::string> content = node.attribute("content");
				// author, changedby, created, changed - LibreOffice 3.5
				// dcterms - old OpenOffice.org
				if (strcasecmp(name.second.c_str(), "author") == 0 ||
						strcasecmp(name.second.c_str(), "dcterms.creator") == 0)
					m_meta.setAuthor(content.second);
				else if (strcasecmp(name.second.c_str(), "changedby") == 0 ||
						strcasecmp(name.second.c_str(), "dcterms.contributor") == 0)
				{
					// Multiple changedby meta tags are possible - LibreOffice 3.5 is an example
					if (std::string(m_meta.lastModifiedBy()).empty())
						m_meta.setLastModifiedBy(content.second);
				}
				else if (strcasecmp(name.second.c_str(), "created") == 0 ||
						strcasecmp(name.second.c_str(), "dcterms.issued") == 0)
				{
					tm creation_date;
					string_to_date(content.second, creation_date);
					m_meta.setCreationDate(creation_date);
				}
				else if (strcasecmp(name.second.c_str(), "changed") == 0 ||
						strcasecmp(name.second.c_str(), "dcterms.modified") == 0)
				{
					// Multiple changed meta tags are possible - LibreOffice 3.5 is an example
					if (m_meta.lastModificationDate().tm_year == 0)
					{
						tm last_modification_date;
						string_to_date(content.second, last_modification_date);
						m_meta.setLastModificationDate(last_modification_date);
					}
				}
			}
		}

	public:
		DocToTextMetaSaxParser(Metadata& meta, std::ostream& log_stream)
			: m_meta(meta), m_log_stream(log_stream)
		{
		};
};

struct HTMLParser::Implementation
{
	bool m_error;
	bool m_skip_decoding;
	std::string m_file_name;
	bool m_verbose_logging;
	std::ostream* m_log_stream;
	DataStream* m_data_stream;
	std::vector<Link> m_links;
};

HTMLParser::HTMLParser(const std::string& file_name)
{
	impl = new Implementation();
	impl->m_error = false;
	impl->m_skip_decoding = false;
	impl->m_file_name = file_name;
	impl->m_verbose_logging = false;
	impl->m_log_stream = &std::cerr;
	impl->m_data_stream = new FileStream(file_name);
}

HTMLParser::HTMLParser(const char *buffer, size_t size)
{
	impl = new Implementation();
	impl->m_error = false;
	impl->m_skip_decoding = false;
	impl->m_file_name = "Memory buffer";
	impl->m_verbose_logging = false;
	impl->m_log_stream = &std::cerr;
	impl->m_data_stream = new BufferStream(buffer, size);
}

HTMLParser::~HTMLParser()
{
	delete impl->m_data_stream;
	delete impl;
}

void HTMLParser::setVerboseLogging(bool verbose)
{
	impl->m_verbose_logging = verbose;
}

void HTMLParser::setLogStream(std::ostream& log_stream)
{
	impl->m_log_stream = &log_stream;
}

bool HTMLParser::isHTML()
{
	impl->m_error = false;
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Error opening file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return false;
	}
	size_t size = impl->m_data_stream->size();
	std::string content(size, 0);
	if (!impl->m_data_stream->read(&content[0], sizeof(unsigned char), size))
	{
		impl->m_data_stream->close();
		*impl->m_log_stream << "Error reading file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return false;
	}
	impl->m_data_stream->close();
	return content.find("<html") != std::string::npos || content.find("<HTML") != std::string::npos;
}

void HTMLParser::getLinks(std::vector<Link>& links)
{
	links = impl->m_links;
}

std::string HTMLParser::plainText(const FormattingStyle& formatting)
{
	impl->m_error = false;
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Error opening file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return "";
	}
	size_t size = impl->m_data_stream->size();
	std::string content(size, 0);
	if (!impl->m_data_stream->read(&content[0], sizeof(unsigned char), size))
	{
		impl->m_data_stream->close();
		*impl->m_log_stream << "Error reading file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return "";
	}
	impl->m_data_stream->close();
	std::string text;
	DocToTextSaxParser parser(content, text, formatting, *impl->m_log_stream, impl->m_links, impl->m_verbose_logging, impl->m_skip_decoding);
	parser.parse(content);
	return text;
}

Metadata HTMLParser::metaData()
{
	impl->m_error = false;
	Metadata meta;
	if (!impl->m_data_stream->open())
	{
		*impl->m_log_stream << "Error opening file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return meta;
	}
	size_t size = impl->m_data_stream->size();
	std::string content(size, 0);
	if (!impl->m_data_stream->read(&content[0], sizeof(unsigned char), size))
	{
		impl->m_data_stream->close();
		*impl->m_log_stream << "Error reading file " << impl->m_file_name << ".\n";
		impl->m_error = true;
		return meta;
	}
	impl->m_data_stream->close();
	DocToTextMetaSaxParser parser(meta, *impl->m_log_stream);
	parser.parse(content);
	return meta;
}

void HTMLParser::skipCharsetDecoding()
{
	impl->m_skip_decoding = true;
}

bool HTMLParser::error()
{
	return impl->m_error;
}

