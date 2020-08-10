#ifndef DOCTOTEXT_ATTACHMENT_H
#define DOCTOTEXT_ATTACHMENT_H

#include <map>
#include <string>
#include "variant.h"

namespace doctotext
{
	/**
		Structure of the attachment in parsed file.
		If parsed file has an attachments, PlainTextExtractor will try to get them.
		Attachment is usually another file. Another thing worth mentioning is that attachments may hold
		metadada.
	**/
	class Attachment
	{
		private:
			struct Implementation;
			Implementation* impl;

		public:
			Attachment();
			Attachment(const std::string& file_name);
			Attachment(const Attachment& attachment);
			Attachment& operator = (const Attachment& attachment);
			~Attachment();
			void setFileName(const std::string& file_name);
			void setBinaryContent(const std::string& binary_content);
			void addField(const std::string& field_name, const Variant& field_value);

			///Gets attachment file name
			const char* filename() const;

			///Gets binary content of the attachment
			const char* binaryContent() const;

			///Returns size of the binary content
			size_t binaryContentSize() const;

			///Checks if metadata for given key exist (for example "Content-Type")
			bool hasField(const std::string& field_name) const;

			///Returns value for given key (metadata)
			const Variant& getField(const std::string& field_name) const;

			///Returns map with all keys and values
			const std::map<std::string, Variant>& getFields() const;
	};
}

#endif
