/**
 * @brief     Wrapper for Compound File Binary Format (CFB)
 * @package   cfb
 * @file      cfb.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.1
 * @date      18.09.2016 -- 29.01.2018
 */
#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>


/**
 * @namespace cfb
 * @brief
 *     Wrapper for Compound File Binary Format (CFB)
 */
namespace cfb {

/**
 * @class Cfb
 * @brief
 *     Wrapper for Compound File Binary Format (CFB)
 */
class Cfb {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Cfb(const std::string& fileName);

	/**
	 * @brief
	 *     Read binary data
	 * @tparam T
	 *     Result data type
	 * @param[in] data
	 *     Binary data
	 * @param[in] offset
	 *     Start position in #data
	 * @param[in] size
	 *     Size of data chunk
	 * @return
	 *     Number value
	 * @since 1.0
	 */
	template<typename T>
	T readByte(const std::string& data, size_t offset, int size) const;

	/**
	 * @brief
	 *     Parse file-system-like structure within a file
	 * @since 1.0
	 */
	void parse();

	/**
	 * @brief
	 *     Get binary stream content by name in "directory" structure
	 * @param[in] name
	 *     Stream name
	 * @param[in] offset
	 *     Start position in #m_fatEntries
	 * @param[in] isRoot
	 *     If stream is root in structure
	 * @return
	 *     Stream content
	 * @since 1.0
	 */
	std::string getStream(const std::string& name, int offset = 0, bool isRoot = false) const;

	/**
	 * @brief
	 *     Clear all data and release resources
	 * @since 1.0
	 */
	void clear();

	/**
	 * @brief
	 *     Decode UTF-16 text
	 * @param[in] data
	 *     input UTF-16 text
	 * @return
	 *     UTF-8 text
	 * @since 1.1
	 */
	static std::string decodeUTF16(const std::string& data);

	/** CFB file name */
	const std::string m_fileName;

protected:
	/**
	 * @brief
	 *     Convert binary data to hex
	 * @param[in] input
	 *     Binary data
	 * @return
	 *     Hex data
	 * @since 1.0
	 */
	static std::string binToHex(const std::string& input);

	/**
	 * @brief
	 *     Convert unicode to UTF-8
	 * @param[in] input
	 *     Unicode string
	 * @param[in] check
	 *     True if should check string before converting
	 * @return
	 *     UTF-8 string
	 * @since 1.1
	 */
	std::string unicodeToUtf8(std::string input, bool check = false) const;

	/** File binary data */
	std::string m_data;
	/** FAT sector size shift (1 << 9 = 512) */
	unsigned short m_sectorShift = 9;
	/** MiniFAT sector size shift (1 << 6 = 64) */
	unsigned short m_miniSectorShift = 6;
	/** Max MiniFAT stream size */
	unsigned short m_miniSectorCutoff = 4096;
	/** FAT-sectors chains */
	std::vector<int> m_fatChains;
	/** FAT-sectors content */
	std::vector<std::pair<std::string, std::map<std::string, int>>> m_fatEntries;
	/** MiniFAT-sectors chains */
	std::vector<int> m_miniFatChains;
	/** MiniFAT-sectors content */
	std::string m_miniFat;

private:
	/**
	 * @brief
	 *     Read data from file header
	 * @since 1.0
	 */
	void handleHeader();

	/**
	 * @brief
	 *     Read DIFAT data (shows sectors with FAT chains description)
	 * @since 1.0
	 */
	void handleDifat();

	/**
	 * @brief
	 *     Transform links to the FAT-sectors into real chains
	 * @since 1.0
	 */
	void handleFatChains();

	/**
	 * @brief
	 *     Read MiniFAT-chains (same as FAT-chains)
	 * @since 1.0
	 */
	void handleMiniFatChains();

	/**
	 * @brief
	 *     Read structure of "files" in current file (all objects of "file system")
	 * @since 1.0
	 */
	void handleDirectoryStructure();

	/**
	 * @brief
	 *     Convert UTF-16 text to ANSI
	 * @param input
	 *     UTF-16 text
	 * @return
	 *     ANSI text
	 */
	std::string utf16ToAnsi(const std::string& input) const;

	/**
	 * @brief
	 *     Update ASCII char code
	 * @param c
	 *     Old char code
	 * @return
	 *     New char code
	 */
	unsigned char hexVal(unsigned char c) const;

	/** CFB version (3 or 4) */
	unsigned short m_version = 3;
	/** Order of bytes in file */
	bool m_isLittleEndian = true;
	/** Amount of files in FAT */
	int m_cDir = 0;
	/** Position of description of first file in FAT */
	int m_fDir = 0;
	/** Amount of FAT-sectors */
	int m_cFAT = 0;
	/** Amount of miniFAT-sectors */
	int m_cMiniFat = 0;
	/** Position of miniFAT-sectors */
	int m_fMiniFat = 0;
	/** DIFAT-sectors data */
	std::vector<int> m_Difat;
	/** Amount of DIFAT-sectors */
	int m_cDifat = 0;
	/** 110 DIFAT-sector offset */
	int m_fDifat = 0;
};


template<typename T>
T Cfb::readByte(const std::string& data, size_t offset, int size) const {
	std::string str = data.substr(offset, size);
	if (m_isLittleEndian)
		std::reverse(str.begin(), str.end());

	union {
		unsigned long long i;
		T t;
	} value;

	value.i = std::stoull(binToHex(str), nullptr, 16);
	return value.t;

	/*unsigned long long i;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> i;
	T t(reinterpret_cast<T&>(i));
	return t;*/
}

template<>
inline std::string Cfb::readByte<std::string>(const std::string& data, size_t offset,
											   int size) const
{
	return data.substr(offset, size);
	/*std::string str = binToHex(data.substr(offset, size));

	std::string out;
	out.reserve(str.length() / 2);
	for (auto p = str.begin(); p != str.end(); p++) {
		unsigned char c = hexVal(*p);
		p++;
		if (p == str.end()) break; // incomplete last digit - should report error
		c = (c << 4) + hexVal(*p); // + takes precedence over <<
		out.push_back(c);
	}
	return out;*/
}

}  // End namespace
