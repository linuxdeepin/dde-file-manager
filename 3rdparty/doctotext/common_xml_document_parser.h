#ifndef DOCTOTEXT_COMMON_XML_PARSER_H
#define DOCTOTEXT_COMMON_XML_PARSER_H

#include "formatting_style.h"
#include "link.h"
#include <string>
#include <vector>
#include <map>

namespace doctotext
{
	class Metadata;
}
class XmlStream;
class DocToTextUnzip;

using namespace doctotext;


/**
	This class is inherited by ODFOOXMLParser and ODFXMLParser. It contains some common
	functions for both parsers.
	How inheritance works:
	Child classes (ODFOOXMLParser and ODFXMLParser for now) may want to add or change handlers for some xml tags
	(using registerODFOOXMLCommandHandler). Besides they have to implement two methods: plainText and metaData.
**/
class CommonXMLDocumentParser
{
	private:
		struct Implementation;
		Implementation* impl;
		class CommandHandlersSet;

	//public interface for derived classes (and its components)
	public:
		enum ODFOOXMLListStyle
		{
			number,
			bullet
		};

		struct Comment
		{
			std::string m_author;
			std::string m_time;
			std::string m_text;
			std::vector<Link> m_links;
			Comment() {}
			Comment(const std::string& author, const std::string& time, const std::string& text, const std::vector<Link>& links)
				: m_author(author), m_time(time), m_text(text), m_links(links) {}
		};

		struct SharedString
		{
			std::string m_text;
			std::vector<Link> m_links;
		};

		typedef std::vector<ODFOOXMLListStyle> ListStyleVector;

		typedef void (*CommandHandler)(CommonXMLDocumentParser& parser, XmlStream& xml_stream, XmlParseMode mode,
									   const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
									   bool& children_processed, std::string& level_suffix, bool first_on_level,
									   std::vector<Link>& links);

		/**
			Each xml tag can have associated handler, which is a single function of CommandHandler type.
			CommonXMLDocumentParser has already a set of functions for some basic tags.
			ODFOOXMLParser and ODFXMLParser can add new/overwrite existing handlers in order to change/extend default behaviour of
			parseXmlData/extractText.
		**/
		void registerODFOOXMLCommandHandler(const std::string& xml_tag, CommandHandler handler);

		///it is executed for each undefined tag (xml tag without associated handler). Can be overwritten
		virtual void onUnregisteredCommand(XmlStream& xml_stream, XmlParseMode mode,
										   const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text,
										   bool& children_processed, std::string& level_suffix, bool first_on_level,
										   std::vector<Link>& links);

		///parses xml data for given xml stream. It executes commands for each xml tag
		std::string parseXmlData(XmlStream& xml_stream, XmlParseMode mode, const FormattingStyle& options, const DocToTextUnzip* zipfile, std::vector<Link>& links);

		///extracts text and links from xml data. It uses parseXmlData internally
		bool extractText(const std::string& xml_contents, XmlParseMode mode, const FormattingStyle& options, const DocToTextUnzip* zipfile, std::string& text, std::vector<Link>& links);

		///usefull since two parsers use this
		bool parseODFMetadata(const std::string &xml_content, Metadata &metadata);

		///this is helpful function to format comment
		const std::string formatComment(const std::string& author, const std::string& time, const std::string& text);

		///Returns information "on how many list objects" we are. Returns 0 if we are not parsing any list actually. Should only be used inside command handlers
		size_t& getListDepth() const;

		///gets vector of links for reading and writing
		std::vector<Link>& getInnerLinks() const;

		///gets list styles for reading and writing
		std::map<std::string, ListStyleVector>& getListStyles() const;

		///gets comments for reading and writing
		std::map<int, Comment>& getComments() const;

		///gets vector of shared strings for reading and writing
		std::vector<SharedString>& getSharedStrings() const;

		///checks if writing to the text is disabled (only inside onUnregisteredCommand!)
		bool disabledText() const;

		///gets options which has been set for XmlStream object. (xmlParserOption from libxml2)
		int getXmlOptions() const;

		bool verbose() const;
		std::ostream& getLogStream() const;
		bool manageXmlParser() const;
		void setError(bool error);

		///disables modifying text data inside method onUnregisteredCommand
		void disableText(bool disable);

		///sets options for XmlStream objects. (xmlParserOption from libxml2)
		void setXmlOptions(int options);

		///Cleans up associated data. Call this method in constructor of derived class in case of bad_alloc.
		void cleanUp();

	//public interface
	public:
		CommonXMLDocumentParser();
		virtual ~CommonXMLDocumentParser();
		void setVerboseLogging(bool verbose);
		void setLogStream(std::ostream& log_stream);
		void setManageXmlParser(bool manage);
		void getLinks(std::vector<Link>& links);
		virtual std::string plainText(XmlParseMode mode, FormattingStyle& options) = 0;
		virtual Metadata metaData() = 0;
		bool error();
};

#endif
