#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DOCTOTEXT_CALL
	#if defined(_WIN32) && !defined(_WIN64)
		#define DOCTOTEXT_CALL __cdecl
	#else
		#define DOCTOTEXT_CALL
	#endif
#endif

typedef int doctotext_parser_type;

#define DOCTOTEXT_PARSER_AUTO 0
#define DOCTOTEXT_PARSER_RTF 1
#define DOCTOTEXT_PARSER_ODF_OOXML 2
#define DOCTOTEXT_PARSER_XLS 3
#define DOCTOTEXT_PARSER_DOC 4
#define DOCTOTEXT_PARSER_PPT 5
#define DOCTOTEXT_PARSER_HTML 6
#define DOCTOTEXT_PARSER_IWORK 7
#define DOCTOTEXT_PARSER_XLSB 8
#define DOCTOTEXT_PARSER_PDF 9
#define DOCTOTEXT_PARSER_TXT 10

typedef int doctotext_xml_parse_mode;

#define DOCTOTEXT_PARSE_XML 0
#define DOCTOTEXT_FIX_XML 1
#define DOCTOTEXT_STRIP_XML 2

typedef int doctotext_table_style;

#define DOCTOTEXT_TABLE_STYLE_TABLE_LOOK 0
#define DOCTOTEXT_TABLE_STYLE_ONE_ROW 1
#define DOCTOTEXT_TABLE_STYLE_ONE_COL 2

typedef int doctotext_url_style;

#define DOCTOTEXT_URL_STYLE_TEXT_ONLY 0
#define DOCTOTEXT_URL_STYLE_EXTENDED 1
#define DOCTOTEXT_URL_STYLE_UNDERSCORED 2

typedef int doctotext_metadata_type;

#define DOCTOTEXT_METADATA_TYPE_NONE 0
#define DOCTOTEXT_METADATA_TYPE_EXTRACTED 1
#define DOCTOTEXT_METADATA_TYPE_ESTIMATED 2

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextFormattingStyle DocToTextFormattingStyle;

/**
 * @brief Creates formatting style object
 * @return Newly created object. Do not forget to delete it after you are done (using doctotext_free_formatting_style).
 * Object is allocated in a C++ way (using operator new), that's why simple "free()" must not be used.
 * DocToTextFormattingStyle object is used to set formatting style that will be used by data extractor.
**/
DocToTextFormattingStyle* DOCTOTEXT_CALL doctotext_create_formatting_style();

/**
 * @brief Releases DocToTextFormattingStyle object. Remember not to use function free(). DocToTextFormattingStyle is
 * allocated using operator new (from C++) and is supposed to be deleted by doctotext_free_formatting_style (which uses
 * operator delete).
 * @param formatting_style - object to be deleted
 */
void DOCTOTEXT_CALL doctotext_free_formatting_style(DocToTextFormattingStyle* formatting_style);

/**
 * @brief Sets style for tables
 * @param formatting_style - DocToTextFormattingStyle object for which we are changing style
 * @param table_style - one of the table styles. Check \c doctotext_table_style
 */
void DOCTOTEXT_CALL doctotext_formatting_style_set_table_style(DocToTextFormattingStyle* formatting_style, doctotext_table_style table_style);

/**
 * @brief Sets style for urls
 * @param formatting_style - DocToTextFormattingStyle object for which we are changing style
 * @param url_style - one of the url styles. Check \c doctotext_url_style
 */
void DOCTOTEXT_CALL doctotext_formatting_style_set_url_style(DocToTextFormattingStyle* formatting_style, doctotext_url_style url_style);

/**
 * @brief Sets prefix which will be placed before every position in the unnumbered list
 * @param formatting_style - DocToTextFormattingStyle object for which we are changing style
 * @param prefix - prefix that should be used in lists
 */
void DOCTOTEXT_CALL doctotext_formatting_style_set_list_prefix(DocToTextFormattingStyle* formatting_style, const char* prefix);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextExtractorParams DocToTextExtractorParams;

/**
 * @brief Creates object which will hold all parameters to use by data extractor. In order to destroy this object,
 * call \c doctotext_free_extractor_params.
 * DocToTextExtractorParams is allocated a in C++ way (using operator new), that's why simple "free()" must not be used.
 * @return Newly created object. Do not forget to delete it after you are done (using doctotext_free_extractor_params)
 */
DocToTextExtractorParams* DOCTOTEXT_CALL doctotext_create_extractor_params();

/**
 * @brief Releases data associated with extractor params object. Remember not to use function free(). DocToTextExtractorParams is
 * allocated using operator new (from C++) and is supposed to be deleted by doctotext_free_extractor_params (which uses
 * operator delete).
 * @param extractor_params - Object to be released
 */
void DOCTOTEXT_CALL doctotext_free_extractor_params(DocToTextExtractorParams* extractor_params);

/**
 * @brief turns on/off verbose logging. If verbose is on (verbose != 0), all additional messages will be logged
 * @param extractor_params - object which holds parameters for data extractor
 * @param verbose - If diffrent than 0, all additional messages will be logged
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_verbose_logging(DocToTextExtractorParams* extractor_params, int verbose);

/**
 * @brief By default all messages are redirected to stderr. But it is possible to redirect those data to another file.
 * @param extractor_params - object which holds parameters for data extractor
 * @param log_file_name - Name of the file to which all logs should be redirected
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_log_file(DocToTextExtractorParams* extractor_params, const char* log_file_name);

/**
 * @brief By default extractor will try to detect parser type manually. But you can change this behaviour
 * @param extractor_params - object which holds parameters for data extractor
 * @param parser_type - type of parser that extractor must use firstly
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_parser_type(DocToTextExtractorParams* extractor_params, doctotext_parser_type parser_type);

/**
 * @brief Sets xml parser mode.
 * @param extractor_params - object which holds parameters for data extractor
 * @param xml_parse_mode - xml parse mode. See \c doctotext_xml_parse_mode
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_xml_parse_mode(DocToTextExtractorParams* extractor_params, doctotext_xml_parse_mode xml_parse_mode);

/**
 * @brief By default doctotext manages libxml2 internally (xmlInitParser() and xmlCleanupParser()).
 * You can disable this behavior, but then you have to call those functions yourself.
 * @param extractor_params - object which holds parameters for data extractor
 * @param manage_xml_parser - Disables (0) or enables (!= 0) managing libxml2 by the extractor
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_manage_xml_parser(DocToTextExtractorParams* extractor_params, int manage_xml_parser);

/**
 * @brief Sets formatting styles that should be applied.
 * @param extractor_params - object which holds parameters for data extractor
 * @param formatting_style - formatting style that must be applied
 */
void DOCTOTEXT_CALL doctotext_extractor_params_set_formatting_style(DocToTextExtractorParams* extractor_params, DocToTextFormattingStyle* formatting_style);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextException DocToTextException;

/**
 * @brief Releases exception object which can be returned by the extractor after parsing file. You must know that
 * DocToTextException is just a simple wrapper for \c Exception class from doctotext. It is allocated by operator
 * new from C++, that's why DocToTextException cannot be released using free(). Always use doctotext_free_exception.
 * @param exception - exception to be released
 */
void DOCTOTEXT_CALL doctotext_free_exception(DocToTextException* exception);

/**
 * @brief Exception just holds information about "backtrace" of error. So, inside we have table
 * of error messages. First error forms a "core" of the error.
 * @param exception - exception object
 * @return number of error messages
 */
size_t DOCTOTEXT_CALL doctotext_exception_error_messages_count(DocToTextException* exception);

/**
 * @brief Returns error message of given index
 * @param exception - exception object
 * @param index_message - index of the error message to returned
 * @return error message
 */
const char* DOCTOTEXT_CALL doctotext_exception_get_error_message(DocToTextException* exception, size_t index_message);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextLink DocToTextLink;

/**
 * @brief This function returns an url for specified link
 * @param link - link object
 * @return url string associated with given link
 */
const char* DOCTOTEXT_CALL doctotext_link_get_url(DocToTextLink* link);

/**
 * @brief Gets the text of the link. Do not be confused url. If you have something like:
 * <a href = "target">link</a>, then text of the link is 'link', and url is 'target'.
 * @param link - link object
 * @return text of the link
 */
const char* DOCTOTEXT_CALL doctotext_link_get_link_text(DocToTextLink* link);

/**
 * @brief Gets offset of the link in the extracted text.
 * @param link - link object
 * @return position of the link in the text.
 */
size_t DOCTOTEXT_CALL doctotext_link_get_link_position(DocToTextLink* link);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextVariant DocToTextVariant;

/**
 * @brief Checks if variant is empty (holds no value)
 * @param variant - variant object
 * @return 1 if variant is empty, 0 otherwise
 */
int DOCTOTEXT_CALL doctotext_variant_is_null(DocToTextVariant* variant);

/**
 * @brief Checks if variant is a string
 * @param variant - variant object
 * @return 1 if variant is a string, 0 otherwise
 */
int DOCTOTEXT_CALL doctotext_variant_is_string(DocToTextVariant* variant);

/**
 * @brief Checks if variant is a number
 * @param variant - variant object
 * @return 1 if variant is a number, 0 otherwise
 */
int DOCTOTEXT_CALL doctotext_variant_is_number(DocToTextVariant* variant);

/**
 * @brief Checks if variant is a date
 * @param variant - variant object
 * @return 1 if variant is a date, 0 otherwise
 */
int DOCTOTEXT_CALL doctotext_variant_is_date_time(DocToTextVariant* variant);

/**
 * @brief Gets string from variant. If variant is not a string, number or date is converted to the string.
 * If variant is empty, empty string is returned
 * @param variant - variant object
 * @return text value
 */
const char* DOCTOTEXT_CALL doctotext_variant_get_string(DocToTextVariant* variant);

/**
 * @brief Gets number from variant. If variant is not a number, 0 is returned
 * @param variant - variant object
 * @return number value
 */
size_t DOCTOTEXT_CALL doctotext_variant_get_number(DocToTextVariant* variant);

/**
 * @brief Gets date from variant. If variant is not a date, "empty" date is returned (object filled with zeros)
 * @param variant - variant object
 * @return date value
 */
const struct tm* DOCTOTEXT_CALL doctotext_variant_get_date_time(DocToTextVariant* variant);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextAttachment DocToTextAttachment;

/**
 * @brief Gets file name of the attachment
 * @param attachment - attachment object
 * @return file name
 */
const char* DOCTOTEXT_CALL doctotext_attachment_get_file_name(DocToTextAttachment* attachment);

/**
 * @brief Gets binary content of the attachment
 * @param attachment - attachment object
 * @return binary data for given attachment
 */
const char* DOCTOTEXT_CALL doctotext_attachment_get_binary_content(DocToTextAttachment* attachment);

/**
 * @brief Gets size of the attachment (size of the binary data)
 * @param attachment - attachment object
 * @return Size of the binary data (which can be obtained using doctotext_attachment_get_binary_content)
 */
size_t DOCTOTEXT_CALL doctotext_attachment_get_binary_content_size(DocToTextAttachment* attachment);

/**
 * @brief Attachment may hold some metadata. All fields are paired like [key - value].
 * This function checks if attachment has a metadata for given key
 * @param attachment - attachment object
 * @param key - key for which we want to check if attachment has a metadata
 * @return 0 if such a value does not exist, 1 otherwise
 */
int DOCTOTEXT_CALL doctotext_attachment_has_field(DocToTextAttachment* attachment, const char* key);

/**
 * @brief Returns metadata value for given key. If value associated with given key
 * does not exist, DocToTextVariant object with a null value is returned (not to be confused with NULL pointer)
 * @param attachment - attachment object
 * @param key - key for which we want to obtain value
 * @return DocToTextVariant object, which can be a date, number, string (or a null)
 */
DocToTextVariant* DOCTOTEXT_CALL doctotext_attachment_get_field(DocToTextAttachment* attachment, const char* key);

/**
 * @brief Returns number of metadata fields in attachment
 * @param attachment - attachment object
 * @return number of metadata fields in the attachment
 */
size_t DOCTOTEXT_CALL doctotext_attachment_fields_count(DocToTextAttachment* attachment);

/**
 * @brief If we want to check all keys for which attachment has metadata, we should use this function.
 * @param attachment - attachment object
 * @return Table of keys (strings) for which values exist
 */
char** DOCTOTEXT_CALL doctotext_attachment_get_keys(DocToTextAttachment* attachment);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextExtractedData DocToTextExtractedData;

/**
 * @brief Releases all extracted data (text, links, attachments). Call this function when you are sure you
 * dont need those data anymore. Since DocToTextExtractedData is allocated using operator new from C++,
 * free() cannot be used to release memory.
 * @param extracted_data - object to be released
 */
void DOCTOTEXT_CALL doctotext_free_extracted_data(DocToTextExtractedData* extracted_data);

/**
 * @brief Gets extracted text data (encoded in UTF-8)
 * @param extracted_data - extracted data object
 * @return Text that was extracted from file
 */
const char* DOCTOTEXT_CALL doctotext_extracted_data_get_text(DocToTextExtractedData* extracted_data);

/**
 * @brief Extractor has ability to extract and gather informations about all links in the given file.
 * We can use this function to obtain links in separated table. For now HTML/EML parser are supported.
 * @param extracted_data - extracted data object
 * @return Table of links
 */
DocToTextLink** DOCTOTEXT_CALL doctotext_extracted_data_get_links(DocToTextExtractedData* extracted_data);

/**
 * @brief Gets number of links that have been extracted from file.
 * @param extracted_data - extracted data object
 * @return Number of extracted links
 */
size_t DOCTOTEXT_CALL doctotext_extracted_data_get_links_count(DocToTextExtractedData* extracted_data);

/**
 * @brief Extractor can get attachments from file that has been parsed. We can obtain table
 * with all attachments using this function. Only EML parser is supported for now
 * @param extracted_data - extracted data object
 * @return Table with all attachments
 */
DocToTextAttachment** DOCTOTEXT_CALL doctotext_extracted_data_get_attachments(DocToTextExtractedData* extracted_data);

/**
 * @brief Gets number of attachments that have been gained
 * @param extracted_data - extracted data object
 * @return Number of attachments in extracted data object
 */
size_t DOCTOTEXT_CALL doctotext_extracted_data_get_attachments_count(DocToTextExtractedData* extracted_data);

/*-------------------------------------------------------------------------------------------------------------------------*/

typedef struct DocToTextMetadata DocToTextMetadata;

/**
 * @brief Releases metadata object. Note that all data associated with metadata will be also deleted
 * (all DocToTextVariant objects, retrieved keys, strings, dates). Also, do not use free() on this object.
 * Only doctotext_free_metadata can be used to free memory. DocToTextMetadata is allocated in a C++ way
 * (using operator new), so it must be released with using of operator delete. This is all done in
 * doctotext_free_metadata.
 * @param metadata - metadata object to be deleted
 */
void DOCTOTEXT_CALL doctotext_free_metadata(DocToTextMetadata* metadata);

/**
 * @brief Returns a name of author of the file that has been parsed
 * @param metadata - metadata object
 * @return string with author of the file
 */
const char* DOCTOTEXT_CALL doctotext_metadata_author(DocToTextMetadata* metadata);

/**
 * @brief Checks if author has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_author_type(DocToTextMetadata* metadata);

/**
 * @brief Returns a name of user who last has modified given file
 * @param metadata - metadata object
 * @return string with user name who last has modifed given file
 */
const char* DOCTOTEXT_CALL doctotext_metadata_last_modify_by(DocToTextMetadata* metadata);

/**
 * @brief Checks if "last modify by" value has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_last_modify_by_type(DocToTextMetadata* metadata);

/**
 * @brief Returns creation date of the parsed file
 * @param metadata - metadata object
 * @return date when file has been created
 */
const struct tm* DOCTOTEXT_CALL doctotext_metadata_creation_date(DocToTextMetadata* metadata);

/**
 * @brief Checks if creation date has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_creation_date_type(DocToTextMetadata* metadata);

/**
 * @brief Returns date when file was last modified
 * @param metadata - metadata object
 * @return date when file has been last modified
 */
const struct tm* DOCTOTEXT_CALL doctotext_metadata_last_modification_date(DocToTextMetadata* metadata);

/**
 * @brief Checks if last modification date has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_last_modification_date_type(DocToTextMetadata* metadata);

/**
 * @brief Returns number of pages in the file
 * @param metadata - metadata object
 * @return number of pages in the file
 */
size_t DOCTOTEXT_CALL doctotext_metadata_pages_count(DocToTextMetadata* metadata);

/**
 * @brief Checks if number of pages has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_pages_count_type(DocToTextMetadata* metadata);

/**
 * @brief Returns number of words in the file
 * @param metadata - metadata object
 * @return number of words in the file
 */
size_t DOCTOTEXT_CALL doctotext_metadata_words_count(DocToTextMetadata* metadata);

/**
 * @brief Checks if number of words has been extracted, only estimated or not parsed at all.
 * @param metadata - metadata object
 * @return extracted/estimated/none - see \c doctotext_metadata_type
 */
doctotext_metadata_type DOCTOTEXT_CALL doctotext_metadata_words_count_type(DocToTextMetadata* metadata);

/**
 * @brief Metadata can hold more additional, custom values. They are paired using pattern [key - value].
 * Note that values for keys like "author", "creation date" etc. are also included
 * This function checks if value of given key exists.
 * @param metadata - metadata object
 * @param key - key for which we want to check if value exist
 * @return 1 if value for a given key exist, 0 otherwise
 */
int DOCTOTEXT_CALL doctotext_metadata_has_field(DocToTextMetadata* metadata, const char* key);

/**
 * @brief Metadata can hold more additional, custom values. They are paired using pattern [key - value].
 * Note that values for keys like "author", "creation date" etc. are also included
 * This function returns a value for given key. If such a value does not exist, DocToTextVariant object
 * with null value is returned.
 * @param metadata - metadata object
 * @param key - key for which we want to get value
 * @return variant object, which can be a date, number or string. Or null if value for given key does not exist.
 * Do not be confused with NULL pointer.
 */
DocToTextVariant* DOCTOTEXT_CALL doctotext_metadata_get_field(DocToTextMetadata* metadata, const char* key);

/**
 * @brief Metadata can hold more additional, custom values. They are paired using pattern [key - value].
 * Note that values for keys like "author", "creation date" etc. are also included
 * This function gets number of all fields in metadata.
 * @param metadata - metadata object
 * @return Number of metadata fields
 */
size_t DOCTOTEXT_CALL doctotext_metadata_fields_count(DocToTextMetadata* metadata);

/**
 * @brief Metadata can hold more additional, custom values. They are paired using pattern [key - value].
 * Note that values for keys like "author", "creation date" etc. are also included
 * This function returns keys for all values
 * @param metadata - metadata object
 * @return Table of keys for all metadata values
 */
char** DOCTOTEXT_CALL doctotext_metadata_get_keys(DocToTextMetadata* metadata);

/*-------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Parses file of given name. Uses DocToTextExtractorParams objects to adjust its behavior to user expectations.
 * Returns object which holds all extracted data (text, links, attachments). Remember to delete returned object after you are done.
 * If some error occurs, this function returns NULL. It may also happen for the correct file, if there was not enough memory
 * @param file_name - file name with path to the file
 * @param extractor_params - object which extractor will use to adjust its behavior to expectations. This is obligatory object.
 * @param exception - If something goes wrong, extractor under the hood may (may, not must!) create exception object which will hold
 * information about the error. If such a thing happens, remember to delete exception object manually. You can pass NULL value
 * here. In that case, exception will never be returned. Do not release this object using free(). Use doctotext_free_exception.
 * DocToTextException is created using operator new, thus cannot be deleted in 'C style'.
 * @return Object with extracted data (text, links, attachments). You are obligated to delete this object manually. But do not use
 * free(). DocToTextExtractedData is C++ object inside and free() will not work. Use \c doctotext_free_extracted_data
 */
DocToTextExtractedData* DOCTOTEXT_CALL doctotext_process_file(const char* file_name, DocToTextExtractorParams* extractor_params, DocToTextException** exception);

/**
 * @brief Parses file from memory buffer. Besides that fact, this function works just like \c doctotext_process_file (from file)
 * @param buffer - buffer with file memory
 * @param size - size of the given buffer
 * @param extractor_params - object which extractor will use to adjust its behavior to expectations. This is obligatory object.
 * @param exception - If something goes wrong, extractor under the hood may (may, not must!) create exception object which will hold
 * information about the error. If such a thing happens, remember to delete exception object manually. You can pass NULL value
 * here. In that case, exception will never be returned. Do not release this object using free(). Use doctotext_free_exception.
 * DocToTextException is created using operator new, thus cannot be deleted in 'C style'.
 * @return Object with extracted data (text, links, attachments). You are obligated to delete this object manually. But do not use
 * free(). DocToTextExtractedData is C++ object inside and free() will not work. Use \c doctotext_free_extracted_data
 */
DocToTextExtractedData* DOCTOTEXT_CALL doctotext_process_file_from_buffer(const char* buffer, size_t size, DocToTextExtractorParams* extractor_params, DocToTextException** exception);

/**
 * @brief Parses file of given name. Uses DocToTextExtractorParams objects to adjust its behavior to user expectations.
 * Returns metadata object (author, creation date etc.). Remember to delete returned object after you are done.
 * If some error occurs, this function returns NULL. It may also happen for the correct file, if there was not enough memory
 * @param file_name - file name with path to the file
 * @param extractor_params - object which extractor will use to adjust its behavior to expectations. This is obligatory object.
 * @param exception - If something goes wrong, extractor under the hood may (may, not must!) create exception object which will hold
 * information about the error. If such a thing happens, remember to delete exception object manually. You can pass NULL value
 * here. In that case, exception will never be returned. Do not release this object using free(). Use doctotext_free_exception.
 * DocToTextException is created using operator new, thus cannot be deleted in 'C style'.
 * @return Object with extracted metadata. You are obligated to delete this object manually. But do not use
 * free(). DocToTextMetadata is C++ object inside and free() will not work. Use \c doctotext_free_metadata
 */
DocToTextMetadata* DOCTOTEXT_CALL doctotext_extract_metadata(const char* file_name, DocToTextExtractorParams* extractor_params, DocToTextException** exception);

/**
 * @brief Parses file from memory buffer. Besides that fact, this function works just like \c doctotext_extract_metadata (from file)
 * @param buffer - buffer with file memory
 * @param size - size of the given buffer
 * @param extractor_params - object which extractor will use to adjust its behavior to expectations. This is obligatory object.
 * @param exception - If something goes wrong, extractor under the hood may (may, not must!) create exception object which will hold
 * information about the error. If such a thing happens, remember to delete exception object manually. You can pass NULL value
 * here. In that case, exception will never be returned. Do not release this object using free(). Use doctotext_free_exception.
 * DocToTextException is created using operator new, thus cannot be deleted in 'C style'..
 * @return Object with extracted metadata. You are obligated to delete this object manually. But do not use
 * free(). DocToTextMetadata is C++ object inside and free() will not work. Use \c doctotext_free_metadata
 */
DocToTextMetadata* DOCTOTEXT_CALL doctotext_extract_metadata_from_buffer(const char* buffer, size_t size, DocToTextExtractorParams* extractor_params, DocToTextException** exception);

/**
 * @brief This function tries to detect parser type for given file. It uses file extension to obtain that information
 * @param file_name - file name with path to the file.
 * @param parser_type - parser type to be returned
 * @return 0 if something goes wrong (practically impossible, only if there was not enough memory to allocate few bytes).
 * 1 otherwise
 */
int DOCTOTEXT_CALL doctotext_parser_type_by_file_extension(const char* file_name, doctotext_parser_type* parser_type);

/**
 * @brief This function tries to detect parser type for given file. It analyzes file content to determine that info.
 * @param file_name - file name with path to the file.
 * @param parser_type - parser type to be returned
 * @return 0 if something goes wrong, 1 otherwise
 */
int DOCTOTEXT_CALL doctotext_parser_type_by_file_content(const char *file_name, doctotext_parser_type *parser_type);

/**
 * @brief This function tries to detect parser type for given file (in memory buffer).
 * It analyzes file content to determine that info.
 * @param buffer - memory buffer with file content
 * @param size - size of the buffer
 * @param parser_type - parser type to be returned
 * @return 0 if something goes wrong, 1 otherwise
 */
int DOCTOTEXT_CALL doctotext_parser_type_by_file_content_from_buffer(const char* buffer, size_t size, doctotext_parser_type* parser_type);

#ifdef __cplusplus
}
#endif
