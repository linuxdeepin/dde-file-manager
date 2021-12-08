/**
 * @brief   Helper functions (strings, files, etc.)
 * @package tools
 * @file    tools.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.3
 * @date    04.09.2016 -- 29.01.2018
 */
#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "pugixml.hpp"


/**
 * @namespace tools
 * @brief
 *     Helper functions (strings, files, etc.)
 */
namespace tools {

	using LOCK = std::unique_lock<std::mutex>;
	extern std::mutex MUTEX;
	/** If current OS is Windows */
	extern const bool IS_WINDOWS;
	/** If current OS is macOS */
	extern const bool IS_MAC;

	/// @name Files
	/// @{
	/**
	 * @brief
	 *     Get current executable file directory to make absolute path
	 * @return
	 *     Current executable file path
	 * @since 1.0
	 */
	std::string getProgramPath();

	/**
	 * @brief
	 *     Get absolute file path
	 * @param[in] fileName
	 *     File name
	 * @return
	 *     Absolute file path
	 * @since 1.0
	 */
	std::string absolutePath(const std::string& fileName);

	/**
	 * @brief
	 *     Rmdir realisation depending on OS
	 * @param[in] dir
	 *     Directory name
	 * @since 1.3
	 */
	void os_rmdir(const char* dir);

	/**
	 * @brief
	 *     Mkdir realisation depending on OS
	 * @param[in] dir
	 *     Directory name
	 * @since 1.0
	 */
	void os_mkdir(const char* dir);

	/**
	 * @brief
	 *     Mkdtemp realisation depending on OS
	 * @param[in] dir
	 *     Directory name
	 * @since 1.3
	 */
	std::string os_mkdtemp(char* dir);

	/**
	 * @brief
	 *     Check if path is directory
	 * @param[in] path
	 *     Directory name
	 * @return
	 *     True if file name is directory
	 * @since 1.0
	 */
	bool isDirectory(const std::string& path);

	/**
	 * @brief
	 *     Recursively create directory
	 * @param[in] name
	 *     Directory name
	 * @since 1.0
	 */
	void createDir(const std::string& name);

	/**
	 * @brief
	 *     Create temp directory
	 * @return
	 *     Temp directory path
	 * @since 1.0
	 */
	std::string createTempDir();

	/**
	 * @brief
	 *     Recursively delete directory
	 * @param[in] name
	 *     Directory name
	 * @param[in] deleteSelf
	 *     False if should clear directory only
	 * @since 1.0
	 */
	void deleteDir(const std::string& name, bool deleteSelf = true);

	/**
	 * @brief
	 *     Check if file exists
	 * @param[in] name
	 *     File name
	 * @return
	 *     True if file exists
	 * @since 1.0
	 */
	bool fileExists(const std::string& name);

	/**
	 * @brief
	 *     Move file
	 * @param[in] oldDir
	 *     Old directory name
	 * @param[in] oldName
	 *     Old file name
	 * @param[in] newDir
	 *     New directory name
	 * @param[in] newName
	 *     New file name
	 * @since 1.0
	 */
	void moveFile(const std::string& oldDir, const std::string& oldName,
				  const std::string& newDir, const std::string& newName);

	/**
	 * @brief
	 *     Get file name from path (without extension)
	 * @param[in] path
	 *     File path
	 * @return
	 *     File name
	 * @since 1.0
	 */
	std::string getFileNameFromPath(const std::string& path);

	/**
	 * @brief
	 *     Get get base directory for 2 files
	 * @param[in] str1
	 *     First file name
	 * @param[in] str2
	 *     Second file name
	 * @return
	 *     Base directory name
	 * @since 1.0
	 */
	std::string getBaseDir(std::string str1, std::string str2);

	/**
	 * @brief
	 *     Get amount of files in directory
	 * @param[in] name
	 *     Directory name
	 * @return
	 *     Amount of files
	 * @since 1.0
	 */
	int getFileCount(const std::string& name);
	/// @}

	/// @name Strings
	/// @{
	/**
	 * @brief
	 *     Remove whitespaces and other characters from both sides of string (std::string)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbols that should be deleted
	 * @return
	 *     Result string
	 * @since 1.0
	 */
	std::string trim(const std::string& str, const std::string& delimiter = " \t\n\r\0\x0B");

	/**
	 * @brief
	 *     Remove whitespaces and other characters from both sides of string (std::wstring)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbols that should be deleted
	 * @return
	 *     Result string
	 * @since 1.0
	 */
	std::wstring trim(const std::wstring& str, const std::wstring& delimiter = L" \t\n\r\0\x0B");

	/**
	 * @brief
	 *     Remove whitespaces and other characters from begining of string
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbols that should be deleted
	 * @return
	 *     Result string
	 * @since 1.0
	 */
	std::string ltrim(const std::string& str, const std::string& delimiter = " \t\n\r\0\x0B");

	/**
	 * @brief
	 *     Remove whitespaces and other characters from end of string
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbols that should be deleted
	 * @return
	 *     Result string
	 * @since 1.0
	 */
	std::string rtrim(const std::string& str, const std::string& delimiter = " \t\n\r\0\x0B");

	/**
	 * @brief
	 *     Pad string to a certain length with another string (from left)
	 * @param[in] str
	 *     Input string
	 * @param[in] length
	 *     Length of padded string
	 * @param[in] filler
	 *     Padding symbol
	 * @return
	 *     Padded string
	 * @since 1.1
	 */
	std::string lpad(const std::string& str, size_t length, char filler);

	/**
	 * @brief
	 *     Pad string to a certain length with another string (from right)
	 * @param[in] str
	 *     Input string
	 * @param[in] length
	 *     Length of padded string
	 * @param[in] filler
	 *     Padding symbol
	 * @return
	 *     Padded string
	 * @since 1.1
	 */
	std::string rpad(const std::string& str, size_t length, char filler);

	/**
	 * @brief
	 *     Replace subtring which starts at `offset` and has size of `length`
	 * @param[in] str
	 *     Input string
	 * @param[in] to
	 *     String to replace the search string with
	 * @param[in] offset
	 *     Substring start position
	 * @param[in] length
	 *     Length of substring to be replaced
	 * @return
	 *     New string
	 * @since 1.1
	 */
	std::string replace(const std::string& str, const std::string& to, size_t offset, size_t length);

	/**
	 * @brief
	 *     Replace all occurrences of substring (std::string)
	 * @param[in,out] str
	 *     Input string
	 * @param[in] from
	 *     String to search for
	 * @param[in] to
	 *     String to replace the search string with
	 * @since 1.0
	 */
	void replaceAll(std::string& str, const std::string& from, const std::string& to);

	/**
	 * @brief
	 *     Replace all occurrences of substring (std::wstring)
	 * @param[in,out] str
	 *     Input string
	 * @param[in] from
	 *     String to search for
	 * @param[in] to
	 *     String to replace the search string with
	 * @since 1.0
	 */
	void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to);

	/**
	 * @brief
	 *     Replace all occurrences of substring
	 * @param[in] str
	 *     Input string
	 * @param[in] from
	 *     String to search for
	 * @param[in] to
	 *     String to replace the search string with
	 * @return
	 *     New string
	 * @since 1.0
	 */
	std::string replaceAll2(std::string str, const std::string& from, const std::string& to);

	/**
	 * @brief
	 *     Find nth occurrence of symbol in string (std::string)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbol that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t findNth(const std::string& str, const char delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Find nth occurrence of symbol in string (std::wstring)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbol that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t findNth(const std::wstring& str, const wchar_t delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Find nth occurrence of substring in string (std::wstring)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Substring that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t findNth(const std::wstring& str, const std::wstring& delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Find nth from right occurrence of symbol in string (std::string)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbol that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t rfindNth(const std::string& str, const char delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Find nth from right occurrence of symbol in string (std::wstring)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Symbol that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t rfindNth(const std::wstring& str, const wchar_t delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Find nth from right occurrence of substring in string (std::wstring)
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Substring that should be found
	 * @param[in] occurrence
	 *     Occurrence number
	 * @return
	 *     Occurrence position
	 * @since 1.0
	 */
	size_t rfindNth(const std::wstring& str, const std::wstring& delimiter, const size_t occurrence);

	/**
	 * @brief
	 *     Repeat string several times
	 * @param[in] str
	 *     Input string
	 * @param[in] times
	 *     How many times string should be repeated
	 * @return
	 *     Result string
	 * @since 1.0
	 */
	std::string repeatString(const std::string& str, int times);

	/**
	 * @brief
	 *     Split string by delimiter
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiter
	 *     Boundary symbol
	 * @return
	 *     Array of strings created by splitting string parameter on boundaries formed by delimiter
	 * @since 1.0
	 */
	std::vector<std::string> explode(const std::string& str, const char delimiter);

	/**
	 * @brief
	 *     Split string by delimiter
	 * @param[in] str
	 *     Input string
	 * @param[in] delimiterList
	 *     Boundary symbols
	 * @param[in] skipEmpty
	 *     True if should skip empty substrings
	 * @return
	 *     Array of strings created by splitting string parameter on boundaries formed by delimiters
	 * @since 1.1
	 */
	std::vector<std::string> explode(std::string str, const std::string& delimiterList,
									 bool skipEmpty = false);

	/**
	 * @brief
	 *     Split string by delimiter
	 * @param[in] strList
	 *     Array of strings
	 * @param[in] delimiter
	 *     Delimiter between strings
	 * @return
	 *     String, which is the concatenation of strings and delimiters
	 * @since 1.1
	 */
	std::string join(const std::vector<std::string>& strList, const std::string& delimiter = "");

	/**
	 * @brief
	 *     Check if string ends with substring
	 * @param[in] str
	 *     Input string
	 * @param[in] ending
	 *     Ending substring
	 * @return
	 *     True if string ends with substring
	 * @since 1.1
	 */
	bool endsWith(const std::string& str, const std::string& ending);

	/**
	 * @brief
	 *     Encrypted / decrypted string with XOR method
	 * @param[in] str
	 *     Input string
	 * @param[in] key
	 *     Cypher key
	 * @return
	 *     Encrypted / decrypted string
	 * @since 1.2
	 */
	std::string xorEncrypt(const std::string& str, const std::string& key);
	/// @}

	/// @name XML
	/// @{
	/**
	 * @brief
	 *     Get XML-node count of children with specific name
	 * @param[in] node
	 *     Input XML-node
	 * @param[in] childName
	 *     Child name
	 * @return
	 *     Children count
	 * @since 1.0
	 */
	int xmlChildrenCount(const pugi::xml_node& node, const std::string& childName = "");

	/**
	 * @brief
	 *     Delete all children
	 * @param[in,out] node
	 *     Input XML-node
	 * @since 1.0
	 */
	void xmlDeleteAllChildren(pugi::xml_node& node);
	/// @}

	/// @name Other
	/// @{
	/**
	 * @brief
	 *     Sleep for N sec
	 * @param[in] time
	 *     Time (in msec)
	 * @since 1.1
	 */
	void sleep(int time);

	/**
	 * @brief
	 *     Get system current time
	 * @param[in] format
	 *     Date format
	 * @return
	 *     Current time
	 * @since 1.0
	 */
	std::string getTime(const char* format = "%d.%m.%Y %H:%M:%S");

	/**
	 * @brief
	 *     Convert number to hex string
	 * @param[in] number
	 *     Input number
	 * @param[in] length
	 *     Length of result string
	 * @return
	 *     Hex string
	 * @since 1.0
	 */
	std::string intToHex(int number, size_t length);

	/**
	 * @brief
	 *     Convert hex char to dec number
	 * @param[in] c
	 *     Hex char
	 * @return
	 *     Dec number
	 * @since 1.0
	 */
	char hexCharToDec(char c);
	/// @}

	/** Current executable file absolute path */
	const std::string PROGRAM_PATH = getProgramPath();
	/** Temp directory path */
	const std::string TEMP_DIR = PROGRAM_PATH + "/files/temp";
	/** Hex string char list */
	const char HEX_DATA[] = "0123456789ABCDEF";

}  // End namespace
