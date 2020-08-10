#ifndef DOCTOTEXT_OSHARED_H
#define DOCTOTEXT_OSHARED_H

#include <iosfwd>

namespace doctotext
{
	class Metadata;
}
class ThreadSafeOLEStorage;

using namespace doctotext;

bool parse_oshared_summary_info(ThreadSafeOLEStorage& storage, std::ostream& log_stream, Metadata& meta);
bool parse_oshared_document_summary_info(ThreadSafeOLEStorage& storage, std::ostream& log_stream, int& slide_count);
bool get_codepage_from_document_summary_info(ThreadSafeOLEStorage& storage, std::ostream& log_stream, std::string& codepage);

#endif
