#ifndef DOCTOTEXT_PLAIN_TEXT_EXTRACTOR_H
#define DOCTOTEXT_PLAIN_TEXT_EXTRACTOR_H

#include "attachment.h"
#include "exception.h"
#include "formatting_style.h"
#include "link.h"
#include <string>
#include <vector>

namespace doctotext
{
	class Metadata;

	/**
		Extracts plain text from documents. In addition it can be used to extract metadata and comments (annotations).
		Example of usage (extracting plain text):
		\code
		PlainTextExtractor extractor;
		std::string text;
		if (extractor.processFile("example.doc", text))
			std::cout << text << std::endl;
		else
			std::cerr << "Error." << std::endl;
		\endcode
		Example of usage (extracting metadata):
		\code
		PlainTextExtractor extractor;
		Metadata meta;
		if (extractor.extractMetadata("example.doc", meta))
			std::cout << meta.author << std::endl;
		else
			std::cerr << "Error." << std::endl;
		\endcode
		Note that each instance of PlainTextExtractor should be used in single thread only. One instance
		of this object cannot parse two or more files in parallel.
	**/
	class PlainTextExtractor
	{
		private:
			struct Implementation;
			Implementation* impl;

		public:

			/**
				Enumerates all supported document formats. \c PARSER_AUTO means unknown format that should be
				determined.
			**/
			enum ParserType
			{
				PARSER_AUTO,
				PARSER_RTF,
				PARSER_ODF_OOXML,
				PARSER_XLS,
				PARSER_DOC,
				PARSER_PPT,
				PARSER_HTML,
				PARSER_IWORK,
				PARSER_XLSB,
				PARSER_PDF,
				PARSER_TXT,
				PARSER_EML,
				PARSER_ODFXML
			};

			/**
				The constructor.
				\param parser_type restricts parser to specified document format. If set to \c PARSER_AUTO the parser
					will work with all supported documents formats.
			**/
			PlainTextExtractor(ParserType parser_type = PARSER_AUTO);

			~PlainTextExtractor();

			/**
				Enables or disables verbose logging. Verbose logging is disabled by default.
				If verbose logging is disabled only important messages and errors are logged.
				If verbose logging is enabled all messages and errors are logged.
				\warning Verbose logging can produce a lot of text, especially if the library was compiled in debug
				mode.
				\param verbose if \c true verbose logging will be enabled. If \c false verbose logging will be
				disabled.
				\see setLogStream
			**/
			void setVerboseLogging(bool verbose);

			/**
				Assign an output stream that will be used for logging messages and errors.
				It can be used to capture logs to a file, string or show them in dialog.
				\c std::cerr stream is used by default.
				\param log_stream the stream that will be used for logging
				\see setVerboseLogging
			**/
			void setLogStream(std::ostream& log_stream);

			/**
				Sets how tables, lists and urls should be formatted in plain text produced by them
				parser.
				\param style instance of structure \c FormattingStyle that specifies formatting style.
				\see FormattingStyle
			**/
			void setFormattingStyle(const FormattingStyle& style);

			void setXmlParseMode(XmlParseMode mode);

			/**
				Enables or disables managing libxml2 parser by the object.
				If it is enabled (default) PlainTextExtractor object calls xmlInitParser() and xmlCleanupParser()
				functions automatically. All PlainTextExtractor objects uses a common thread-safe counter for this
				purpose. This is good if you are not using libxml2 elsewhere in the application.
				It it is disabled it is your responsibility to call xmlInitParser() and xmlCleanupParser().
				\param manage if \c true managing will be enabled, if \c false managing will be disabled.
			**/
			void setManageXmlParser(bool manage);

			/**
				Tries to determine document format by file name extension.
				\warning Some applications save CSV documents with "xls" extension, RTF documents with "doc"
				extension or HTML documents with "xls" or "doc" extension. In such a situation this simple test
				will fail.
				\param file_name file name or full path to file.
				\return value of \c ParserType type representing determined document format or PARSER_AUTO if
				document format cannot be determined.
				\see ParserType parserTypeByFileContent
			**/
			ParserType parserTypeByFileExtension(const std::string& file_name);

			/**
				\overload
			**/
			ParserType parserTypeByFileExtension(const char* file_name);

			/**
				Tries to determine document format by file content.
				\param file_name full path to file containing document.
				\param reference to variable of \c ParserType type that will contain determined document format
				or PARSER_AUTO if document format cannot be determined.
				\return \c true if document was processed successfully, \c false otherwise.
				\see ParserType parserTypeByFileExtension
			**/
			bool parserTypeByFileContent(const std::string& file_name, ParserType& parser_type);

			/**
				\overload
			**/
			bool parserTypeByFileContent(const char* file_name, ParserType& parser_type);

			/**
				Tries to determine document format by file content. Uses memory buffer instead of file.
				\overload
				\param buffer pointer to the file content array
				\param size size of buffer
			**/
			bool parserTypeByFileContent(const char* buffer, size_t size, ParserType& parser_type);

			/**
				Parses specified document and extracts plain text.
				\param file_name full path to file containing document.
				\param text reference to object of \c std::string class that will contain produced plain text.
				\return \c true if document was processed successfully, \c false otherwise.
				\see ParserType setFormattingStyle
			**/
			bool processFile(const std::string& file_name, std::string& text);

			/**
				\overload
				\param text reference to pointer that will point to produced plain text in form of null-terminated
					array of chars. The caller is responsible for deleting the buffer using \c delete[] operator.
			**/
			bool processFile(const char* file_name, char*& text);

			/**
				Parses specified document and extracts plain text. Uses memory buffer instead of file.
				\overload
				\param buffer pointer to the file content array
				\param size size of buffer
			**/
			bool processFile(const char* buffer, size_t size, char*& text);

			/**
				\overload
			**/
			bool processFile(const char* buffer, size_t size, std::string& text);

			/**
				Parses specified document and extracts plain text.
				\param parser_type restricts parser to specified document format. If set to \c PARSER_AUTO the parser
					will work with all supported documents formats. This argument override parser type set for
					the object.
				\param fallback if \c true parser will try to detect document format if parsing of document format
					specified in \c parser_type argument fails. This parameter is ignored if \c parser_type is
					set to \c PARSER_AUTO.
				\param file_name full path to file containing document.
				\param text reference to object of \c std::string class that will contain produced plain text.
				\return \c true if document was processed successfully, \c false otherwise.
				\see ParserType setFormattingStyle
			**/
			bool processFile(ParserType parser_type, bool fallback, const std::string& file_name, std::string& text);

			/**
				\overload
				\param text reference to pointer that will point to produced plain text in form of null-terminated
					buffer. The caller is responsible for deleting the buffer using \c delete[] operator.
			**/
			bool processFile(ParserType parser_type, bool fallback, const char* file_name, char*& text);

			/**
				Parses specified document and extracts plain text. Uses memory buffer instead of file.
				\overload
				\param buffer pointer to the file content array
				\param size size of buffer
			**/
			bool processFile(ParserType parser_type, bool fallback, const char* buffer, size_t size, char*& text);

			/**
				\overload
			**/
			bool processFile(ParserType parser_type, bool fallback, const char* buffer, size_t size, std::string& text);

			/**
				Parses specified document and extracts metadata (author, creation time, etc).
				\param file_name full path to file containing document.
				\param metadata reference to object of \c Metadata class that will contain extracted information.
				\return \c true if document was processed successfully, \c false otherwise.
				\see ParserType Metadata
			**/
			bool extractMetadata(const std::string& file_name, Metadata& metadata);

			/**
				\overload
			**/
			bool extractMetadata(const char* file_name, Metadata& metadata);

			/**
				Parses specified document and extracts metadata (author, creation time, etc). Uses memory buffer instead of file.
				\overload
				\param buffer pointer to the file content array
				\param size size of buffer
			**/
			bool extractMetadata(const char* buffer, size_t size, Metadata& metadata);

			/**
				Parses specified document and extracts metadata (author, creation time, etc).
				\param parser_type restricts parser to specified document format. If set to \c PARSER_AUTO the parser
					will work with all supported documents formats. This argument override parser type set for
					the object.
				\param fallback if \c true parser will try to detect document format if parsing of document format
					specified in \c parser_type argument fails. This parameter is ignored if \c parser_type is
					set to \c PARSER_AUTO.
				\param file_name full path to file containing document.
				\param metadata reference to object of \c Metadata class that will contain extracted information.
				\return \c true if document was processed successfully, \c false otherwise.
				\see ParserType Metadata
			**/
			bool extractMetadata(ParserType parser_type, bool fallback, const std::string& file_name, Metadata& metadata);

			/**
				\overload
			**/
			bool extractMetadata(ParserType parser_type, bool fallback, const char* file_name, Metadata& metadata);

			/**
				Parses specified document and extracts metadata (author, creation time, etc). Uses memory buffer instead of file.
				\overload
				\param buffer pointer to the file content array
				\param size size of buffer
			**/
			bool extractMetadata(ParserType parser_type, bool fallback, const char* buffer, size_t size, Metadata& metadata);

			/**
				Gets number of links in the last parsed file. Supported parsers: HTML/EML/ODF_OOXML/ODFXML.
				\return number of the links in parsed file.
				\see Link
			**/
			size_t getNumberOfLinks() const;

			/**
				Gets vector of the links in the last parsed file. Supported parsers: HTML/EML/ODF_OOXML/ODFXML.
				\see Link
			**/
			void getParsedLinks(std::vector<Link>& links) const;

			/**
				Gets table of the links in the last parsed file. Supported parsers: HTML/EML/ODF_OOXML/ODFXML.
				Note that table of links will be deleted, if \c PlainTextExtractor is deleted or another file is parsed.
				\overload
				\param links pointer to the first element in table of \c Link objects.
				\param number_of_links number of links in table.
				\see Link
			**/
			void getParsedLinks(const Link*& links, size_t& number_of_links) const;

			/**
				Gets table of the links in the last parsed file. Supported parsers: HTML/EML/ODF_OOXML/ODFXML.
				Note that table of links will be deleted, if PlainTextExtractor is deleted or another file is parsed.
				\overload
				\return pointer to the first element in table of \c Link objects.
				\see Link
			**/
			const Link* getParsedLinks() const;

			/**
				Gets vector of the attachments in the last parsed file. Only EML parser is supported for now.
				Call this method after you have processed file.
				\see Attachment
			**/
			void getAttachments(std::vector<Attachment>& attachments) const;

			/**
				Gets table of the attachments in the last parsed file. Only EML parser is supported for now.
				Call this method after you have processed file.
				Note that table of attachments will be deleted, if \c PlainTextExtractor is deleted or another file is parsed.
				\overload
				\param attachments pointer to the first element in table of \c Attachment objects.
				\param number_of_attachments number of attachments in table.
				\see Attachment
			**/
			void getAttachments(const Attachment*& attachments, size_t& number_of_attachments) const;

			/**
				Gets table of the attachments in the last parsed file. Only EML parser is supported for now.
				Call this method after you have processed file.
				Note that table of attachments will be deleted, if PlainTextExtractor is deleted or another file is parsed.
				\overload
				\return pointer to the first element in table of \c Attachment objects.
				\see Attachment
			**/
			const Attachment* getAttachments() const;

			/**
				Gets number of attachments in the last parsed file. Only EML parser is supported for now.
				Call this method after you have processed file.
				\return number of the attachments in parsed file.
				\see Attachment
			**/
			size_t getNumberOfAttachments() const;
	};
}

#endif
