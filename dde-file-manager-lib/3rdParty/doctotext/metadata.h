#ifndef DOCTOTEXT_METADATA_H
#define DOCTOTEXT_METADATA_H

#include <string>
#include <map>
#include "variant.h"

namespace doctotext
{
	class Metadata
	{
		private:
			struct Implementation;
			Implementation* impl;

		public:
			enum DataType { NONE, EXTRACTED, ESTIMATED };
			Metadata();
			Metadata(const Metadata& r);
			~Metadata();
			Metadata& operator=(const Metadata& r);

			DataType authorType();
			void setAuthorType(DataType type);

			const char* author();
			void setAuthor(const std::string& author);

			DataType creationDateType();
			void setCreationDateType(DataType type);

			const tm& creationDate();
			void setCreationDate(const tm& creation_date);

			DataType lastModifiedByType();
			void setLastModifiedByType(DataType type);

			const char* lastModifiedBy();
			void setLastModifiedBy(const std::string& last_modified_by);

			DataType lastModificationDateType();
			void setLastModificationDateType(DataType type);

			const tm& lastModificationDate();
			void setLastModificationDate(const tm& last_modification_date);

			DataType pageCountType();
			void setPageCountType(DataType type);

			int pageCount();
			void setPageCount(int page_count);

			DataType wordCountType();
			void setWordCountType(DataType type);

			int wordCount();
			void setWordCount(int word_count);

			void addField(const std::string& field_name, const Variant& field_value);
			bool hasField(const std::string& field_name) const;
			const Variant& getField(const std::string& field_name) const;

			const std::map<std::string, Variant>& getFields() const;
	};
}

#endif
