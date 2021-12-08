/**
 * @brief     Wrapper for Compound File Binary Format (CFB)
 * @package   cfb
 * @file      cfb.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.1
 * @date      18.09.2016 -- 28.01.2018
 */
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "encoding.cpp"
#include "tools.hpp"

#include "cfb.hpp"


namespace cfb {

/** End of chain value */
const int END_OF_CHAIN = 0xFFFFFFFE;
/** Free sector value */
const int FREE_SECTOR   = 0xFFFFFFFF;
/** Hex string char list */
const char HEX_DATA[] = "0123456789ABCDEF";

// public:
Cfb::Cfb(const std::string& fileName)
	: m_fileName(fileName) {}

void Cfb::parse() {
	std::ifstream inputFile(m_fileName, std::ios::binary);
	m_data.assign(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
	inputFile.close();

	// Check CFB 8 bytes signature (widespread and deprecated)
	auto abSig = binToHex(readByte<std::string>(m_data, 0, 8));
	if (abSig != "D0CF11E0A1B11AE1" && abSig != "0E11FC0DD0CF11E0")
		return;

	// Handle file parts
	handleHeader();
	handleDifat();
	handleFatChains();
	handleMiniFatChains();
	handleDirectoryStructure();

	// Root stream should be present necessarily in file structure
    m_miniFat = getStream("Root Entry",0, true);
	if (m_miniFat.empty())
		return;
	// Delete unused link to the DIFAT-sector
	m_Difat.clear();
}

std::string Cfb::getStream(const std::string& name, int offset, bool isRoot) const {
	size_t fatEntriesSize = m_fatEntries.size();
	for (size_t id = offset; id < fatEntriesSize; id++) {
		if (m_fatEntries[id].first == name) {
			std::string stream;
			int start = m_fatEntries.at(id).second.at("start");
			int size  = m_fatEntries.at(id).second.at("size");

			// Situation 1: size < 4096 bytes => read data from MiniFAT
			if (size < m_miniSectorCutoff && !isRoot) {
				// Get size of miniFAT sector (64 bytes)
				int sectorSize = 1 << m_miniSectorShift;
				do {
                    offset  = start << m_miniSectorShift;
					stream += m_miniFat.substr(offset, sectorSize);
					start   = (start < static_cast<int>(m_miniFatChains.size()))
							  ? m_miniFatChains[start]
							  : END_OF_CHAIN;
				} while (start != END_OF_CHAIN);
			}
			// Situation 2: size > 4096 bytes => read data from FAT
			else {
				// Get size of sector (512/4096 bytes)
				int sectorSize = 1 << m_sectorShift;
				do {
                    if(start<0){
                        start =0; //start<0 会崩溃
                    }
					offset  = (start + 1) << m_sectorShift;
					stream += m_data.substr(offset, sectorSize);
					start   = (start < static_cast<int>(m_fatChains.size()))
							  ? m_fatChains[start]
							  : END_OF_CHAIN;
				} while (start != END_OF_CHAIN);
			}
			return stream.substr(0, size);
		}
	}
	return "";
}

void Cfb::clear() {
	m_data.clear();
	m_fatChains.clear();
	m_fatEntries.clear();
	m_miniFatChains.clear();
	m_miniFat.clear();
	m_Difat.clear();
	// Release memory
	m_data.shrink_to_fit();
	m_fatChains.shrink_to_fit();
	m_fatEntries.shrink_to_fit();
	m_miniFatChains.shrink_to_fit();
	m_miniFat.shrink_to_fit();
	m_Difat.shrink_to_fit();
}

std::string Cfb::decodeUTF16(const std::string& data) {
	std::string input = binToHex(data);
	std::string out;

	size_t size = input.size();
	for (size_t i = 0; i < size; i += 4) {
		unsigned int code;
		std::stringstream ss;
		ss << std::hex << input.substr(i, 4);
		ss >> code;

		if (code <= 0x7f) {
			out.append(1, static_cast<char>(code));
		}
		else if (code <= 0x7ff) {
			out.append(1, static_cast<char>(0xc0 | ((code >> 6) & 0x1f)));
			out.append(1, static_cast<char>(0x80 | (code & 0x3f)));
		}
		else if (code <= 0xffff) {
			out.append(1, static_cast<char>(0xe0 | ((code >> 12) & 0x0f)));
			out.append(1, static_cast<char>(0x80 | ((code >> 6) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | (code & 0x3f)));
		}
		else {
			out.append(1, static_cast<char>(0xf0 | ((code >> 18) & 0x07)));
			out.append(1, static_cast<char>(0x80 | ((code >> 12) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | ((code >> 6) & 0x3f)));
			out.append(1, static_cast<char>(0x80 | (code & 0x3f)));
		}
	}

	return out;
}


// protected:
std::string Cfb::binToHex(const std::string& input) {
	std::string out;
	for (auto sc : input) {
		unsigned char c = static_cast<unsigned char>(sc);
		out += HEX_DATA[c >> 4];
		out += HEX_DATA[c & 0xf];
	}
	return out;
}

std::string Cfb::unicodeToUtf8(std::string input, bool check) const {
	std::string out = "";
	if (check && input.find('\0') != std::string::npos) {
		size_t i;
		while ((i = input.find(0x13)) != std::string::npos) {
			size_t j = input.find(0x15, i + 1);
			if (j == std::string::npos)
				break;
			input = tools::replace(input, "", i, j - i);
		}
		for (size_t j = 0; j < input.size(); ++j) {
			if (!(input[j] >= 32 || input[j] == ' ' || input[j] == '\n'))
				input = tools::replace(input, "", i, 1);
		}
		tools::replaceAll(input, "\0", "");
		return input;
	}
	else if (check) {
		size_t i;
		while ((i = input.find("\x13\0")) != std::string::npos) {
			auto j = input.find("\x13\0", i + 1);
			if (j == std::string::npos)
				break;
			input = tools::replace(input, "", i, j - i);
		}
		tools::replaceAll(input, "\0\0", "");
	}
	// Iterate through 2-bytes sequences
	bool skip = false;
	for (size_t i = 0; i < input.size(); i += 2) {
		std::string cd = input.substr(i, 2);
		if (skip) {
			if (cd[1] == 0x15 || cd[0] == 0x15)
				skip = false;
			continue;
		}
		// If last byte (little-endian) is 0x00 => ANSI
		if (cd[1] == 0) {
			if (cd[0] >= 32 || cd[0] == ' ' || cd[0] == '\n') {
				out += cd[0];
			}
			else if (cd[0] == 0x13) {
				skip = true;
			}
			else {
				//continue;
				switch (cd[0]) {
					case 0x0D: case 0x07:
						out += "\n"; break;
					case 0x08: case 0x01:
						out += ""; break;
					case 0x13:
						out += "HYPER13"; break;
					case 0x14:
						out += "HYPER14"; break;
					case 0x15:
						out += "HYPER15"; break;
					default:
						out += " "; break;
				}
			}
		}
		// Otherwise => HTML entity
		else {
			if (cd[1] == 0x13) {
				skip = true;
				continue;
			}
			//out += "&#x" + tools::intToHex(readByte<unsigned short>(cd, 0, 2), 4) + ";";
			unsigned short code = readByte<unsigned short>(cd, 0, 2);
			out += encoding::htmlSpecialDecode(tools::intToHex(code, 4));
		}
	}
	return out;
}


// private:
void Cfb::handleHeader() {
	m_isLittleEndian   = (binToHex(readByte<std::string>(m_data, 0x1C, 2)) == "FEFF");
	m_version          = readByte<unsigned short>(m_data, 0x1A, 2);
	m_sectorShift      = readByte<unsigned short>(m_data, 0x1E, 2);
	m_miniSectorShift  = readByte<unsigned short>(m_data, 0x20, 2);
	m_miniSectorCutoff = readByte<unsigned short>(m_data, 0x38, 2);

	m_cDir     = (m_version == 4) ? readByte<int>(m_data, 0x28, 4) : 0;
	m_fDir     = readByte<int>(m_data, 0x30, 4);
	m_cFAT     = readByte<int>(m_data, 0x2C, 4);
	m_cMiniFat = readByte<int>(m_data, 0x40, 4);
	m_fMiniFat = readByte<int>(m_data, 0x3C, 4);
	m_cDifat   = readByte<int>(m_data, 0x48, 4);
	m_fDifat   = readByte<int>(m_data, 0x44, 4);
}

void Cfb::handleDifat() {
	// First 109 links to the chains are stored in header
	for (int i = 0; i < 109; i++)
		m_Difat.emplace_back(readByte<int>(m_data, 0x4C + i*4, 4));
	// Searching for links to the chains in files > 8,5 Mb
	if (m_fDifat != END_OF_CHAIN) {
		int offset = m_fDifat;
		int size   = 1 << m_sectorShift;
		int i, j   = 0;

		do {
			int start = (offset + 1) << m_sectorShift;
			for (i = 0; i < (size - 4); i += 4)
				m_Difat.emplace_back(readByte<int>(m_data, start + i, 4));
			// Link to the next DIFAT-sector is in the last "word" in current DIFAT-sector
			offset = readByte<int>(m_data, start + i, 4);
		} while (offset != END_OF_CHAIN && ++j < m_cDifat);
	}

	// Delete unused links
    int difat = m_Difat.back();
    while (difat == FREE_SECTOR || difat == END_OF_CHAIN) {
		m_Difat.pop_back();
        difat = m_Difat.back();
    }
}

void Cfb::handleFatChains() {
	int size = 1 << m_sectorShift;
	for (auto const & df : m_Difat) {
		int offset = (df + 1) << m_sectorShift;
		// Get FAT-chain: index - current sector, value - index of next element
		for (int j = 0; j < size; j += 4)
			m_fatChains.emplace_back(readByte<int>(m_data, offset + j, 4));
	}
}

void Cfb::handleMiniFatChains() {
	int offset = m_fMiniFat;
	int size   = 1 << m_sectorShift;
	// If file is used MiniFAT
	while (offset != END_OF_CHAIN) {
		int start = (offset + 1) << m_sectorShift;
		// Read chain from current sector
		for (int i = 0; i < size; i += 4)
			m_miniFatChains.emplace_back(readByte<int>(m_data, start + i, 4));
		offset = (offset < static_cast<int>(m_fatChains.size()))
				 ? m_fatChains[offset]
				 : END_OF_CHAIN;
	}
}

void Cfb::handleDirectoryStructure() {
	int offset = m_fDir;
	int size   = 1 << m_sectorShift;
	do {
		int start = (offset + 1) << m_sectorShift;
		// Read 4/128 entrances in each sector
		for (int i = 0; i < size; i += 128) {
			auto entry = readByte<std::string>(m_data, start + i, 128);
			auto sz    = readByte<unsigned short>(entry, 0x40, 2) - 2;
			m_fatEntries.push_back({
				utf16ToAnsi(readByte<std::string>(entry, 0, sz)), {
					{"type",  entry[0x42]},  // Type (stream, user data, ...)
					{"color", entry[0x43]},  // Color in Red-Black tree
					{"left",  readByte<int>(entry, 0x44, 4)},  // Left elements
					{"right", readByte<int>(entry, 0x48, 4)},  // Right elements
					{"child", readByte<int>(entry, 0x4C, 4)},  // Child element
					{"start", readByte<int>(entry, 0x74, 4)},  // Offset in FAT or miniFAT
					{"size",  readByte<int>(entry, 0x78, 8)}   // Data size (long long)
				}
			});
		}
		// Get next sector with descriptions (if exists)
		offset = (offset < static_cast<int>(m_fatChains.size()))
				 ? m_fatChains[offset]
				 : END_OF_CHAIN;
	} while (offset != END_OF_CHAIN);

	// Delete empty entrances
	while (m_fatEntries.back().second["type"] == 0)
		m_fatEntries.pop_back();
}

std::string Cfb::utf16ToAnsi(const std::string& input) const {
	std::string out;
	int size = static_cast<int>(input.size());
	for (int i = 0; i < size; i += 2)
		out += static_cast<char>(readByte<unsigned short>(input, i, 2));
	return out;
}

unsigned char Cfb::hexVal(unsigned char c) const {
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	else if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	else
		return 0;
}

}  // End namespace
