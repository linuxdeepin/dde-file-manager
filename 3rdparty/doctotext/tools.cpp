/**
 * @brief   Helper functions (strings, files, etc.)
 * @package tools
 * @file    tools.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    04.09.2016 -- 29.01.2018
 */
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <thread>

#include "tools.hpp"


namespace tools {

std::mutex MUTEX;

#if defined(_WIN32) || defined(_WIN64)
	#include <direct.h>
	#include <io.h>
	#include <windows.h>
	#include "libs/dirent.h"

	const bool IS_WINDOWS = true;
	const bool IS_MAC = false;

	std::string absolutePath(const std::string &fileName) {
		/*TCHAR fullPath[MAX_PATH];
		GetFullPathName(fileName.c_str(), MAX_PATH, fullPath, NULL);
		return fullPath;*/
		char fullPath[_MAX_PATH];
		_fullpath(fullPath, fileName.c_str(), _MAX_PATH);
		return fullPath;
	}

	void os_rmdir(const char* dir) { _rmdir(dir); }
	void os_mkdir(const char* dir) {
		/*CreateDirectory(dir, NULL);*/
		_mkdir(dir);
	}
	std::string os_mkdtemp(char* dir) { return _mktemp(dir); }
#else
	#include <dirent.h>
	#include <limits.h>
	#include <stdlib.h>
	#include <unistd.h>

	const bool IS_WINDOWS = false;
	#if defined(__APPLE__)
		const bool IS_MAC = true;
	#else
		const bool IS_MAC = false;
	#endif

	std::string absolutePath(const std::string &fileName) {
		char fullPath[PATH_MAX];
		realpath(fileName.c_str(), fullPath);
		return fullPath;
	}

	void os_rmdir(const char* dir) { rmdir(dir); }
	void os_mkdir(const char* dir) { mkdir(dir, S_IRWXU); }
	std::string os_mkdtemp(char* dir) { return mkdtemp(dir); }
#endif

// Files
std::string getProgramPath() {
	char currentPath[FILENAME_MAX];
	getcwd(currentPath, sizeof(currentPath));
	return currentPath;
}

bool isDirectory(const std::string& path) {
	struct stat fileInfo;
	stat(path.c_str(), &fileInfo);
	return S_ISDIR(fileInfo.st_mode);
}

void createDir(const std::string& name) {
	char  path[PATH_MAX];
	char* p;

	// Copy string so its mutable
	strcpy(path, name.c_str());
	// Iterate the string
	for (p = path + 1; *p; p++) {
		if (*p == '/') {
			// Temporarily truncate
			*p = '\0';
			os_mkdir(path);
			*p = '/';
		}
	}
	os_mkdir(path);
}

std::string createTempDir() {
	std::string mask = TEMP_DIR + "/XXXXXX";
	createDir(TEMP_DIR);
	return os_mkdtemp(&mask[0]);
}

void deleteDir(const std::string& name, bool deleteSelf) {
	DIR* dp = opendir(name.c_str());
	struct dirent *dirp;
	if (dp) {
		while ((dirp = readdir(dp))) {
			if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
				std::string path = name +"/"+ dirp->d_name;
				if (isDirectory(path))
					deleteDir(path, true);
				else
					unlink(path.c_str());
			}
		}
		closedir(dp);
	}

	if (deleteSelf)
		os_rmdir(name.c_str());
}

bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void moveFile(const std::string& oldDir, const std::string& oldName,
			  const std::string& newDir, const std::string& newName)
{
	createDir(newDir + "/Converted");
	std::rename((oldDir +"/"+ oldName).c_str(), (newDir +"/"+ newName).c_str());
}

std::string getFileNameFromPath(const std::string& path) {
	auto first = path.find_last_of("/\\") + 1;
	if (first == std::string::npos) return "";
	auto last = path.find_last_of(".");

	return path.substr(first, (last-first));
}

std::string getBaseDir(std::string str1, std::string str2) {
	std::string result;
	std::vector<std::string> v1 = explode(str1, '/');
	std::vector<std::string> v2 = explode(str2, '/');
	size_t size = (std::min)(v1.size(), v2.size());

	for (size_t i = 0; i < size; ++i) {
		if (v1[i] == v2[i])
			result += v1[i] + "/";
	}
	return result.substr(0, result.size() - 1);
}

int getFileCount(const std::string& name) {
	DIR* dp = opendir(name.c_str());
	struct dirent *dirp;
	int count = 0;
	if (dp) {
		while ((dirp = readdir(dp))) {
			if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..") &&
				!isDirectory(name +"/"+ dirp->d_name))
				count++;
		}
		closedir(dp);
	}
	return count;
}


// Strings
std::string trim(const std::string& str, const std::string& delimiter) {
	auto first = str.find_first_not_of(delimiter);
	if (first == std::string::npos) return "";
	auto last = str.find_last_not_of(delimiter);

	return str.substr(first, (last-first+1));
}

std::wstring trim(const std::wstring& str, const std::wstring& delimiter) {
	auto first = str.find_first_not_of(delimiter);
	if (first == std::string::npos) return L"";
	auto last = str.find_last_not_of(delimiter);

	return str.substr(first, (last-first+1));
}

std::string ltrim(const std::string& str, const std::string& delimiter) {
	auto first = str.find_first_not_of(delimiter);
	if (first == std::string::npos) return "";

	return str.substr(first);
}

std::string rtrim(const std::string& str, const std::string& delimiter) {
	auto last = str.find_last_not_of(delimiter);
	if (last == std::string::npos) return "";

	return str.substr(0, last+1);
}

std::string lpad(const std::string& str, size_t length, char filler) {
	if (length > str.size())
		return str + std::string(length - str.size(), filler);
	else
		return str;
}

std::string rpad(const std::string& str, size_t length, char filler) {
	if (length > str.size())
		return std::string(length - str.size(), filler) + str;
	else
		return str;
}

std::string replace(const std::string& str, const std::string& to, size_t offset, size_t length) {
	return str.substr(0, offset - 1) + to + str.substr(offset + length);
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start = 0;
	while((start = str.find(from, start)) != std::string::npos) {
		str.replace(start, from.length(), to);
		// If `to` contains `from` (like replacing 'x' with 'yx')
		start += to.length();
	}
}

void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
	if (from.empty())
		return;
	size_t start = 0;
	while((start = str.find(from, start)) != std::string::npos) {
		str.replace(start, from.length(), to);
		// If `to` contains `from` (like replacing 'x' with 'yx')
		start += to.length();
	}
}

std::string replaceAll2(std::string str, const std::string& from, const std::string& to) {
	if (from.empty())
		return str;
	size_t start = 0;
	while((start = str.find(from, start)) != std::string::npos) {
		str.replace(start, from.length(), to);
		// If `to` contains `from` (like replacing 'x' with 'yx')
		start += to.length();
	}
	return str;
}

size_t findNth(const std::string& str, const char delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return 0;
	size_t res = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == delimiter)
			res++;
		if (res == occurrence)
			return i;
	}
	return str.size();
}

size_t findNth(const std::wstring& str, const wchar_t delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return 0;
	size_t res = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == delimiter)
			res++;
		if (res == occurrence)
			return i;
	}
	return str.size();
}

size_t findNth(const std::wstring& str, const std::wstring& delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return 0;
	size_t pos  = 0;
	size_t from = 0;
	size_t size = delimiter.size();
	size_t i = 0;
	while (i < occurrence) {
		pos = str.find(delimiter, from);
		if (std::wstring::npos == pos)
			return str.size();
		from = pos + size;
		++i;
	}
	return pos;
}

size_t rfindNth(const std::string& str, const char delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return str.size();
	size_t res = 0;
	for (int i = static_cast<int>(str.size()); i >= 0; --i) {
		if (str[i] == delimiter)
			res++;
		if (res == occurrence)
			return i;
	}
	return 0;
}

size_t rfindNth(const std::wstring& str, const wchar_t delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return str.size();
	size_t res = 0;
	for (int i = static_cast<int>(str.size()); i >= 0; --i) {
		if (str[i] == delimiter)
			res++;
		if (res == occurrence)
			return i;
	}
	return 0;
}

size_t rfindNth(const std::wstring& str, const std::wstring& delimiter, const size_t occurrence) {
	if (occurrence <= 0)
		return 0;
	size_t pos = 0;
	size_t to  = str.size();
	size_t res = 0;
	while (res < occurrence) {
		pos = str.rfind(delimiter, to);
		if (std::string::npos == pos)
			return 0;
		to = pos - 1;
		++res;
	}
	return pos;
}

std::string repeatString(const std::string & word, int times) {
	std::string result;
	result.reserve(times * word.length()); // avoid repeated reallocation
	for (int i = 0; i < times; i++)
		result += word;
	return result;
}

std::vector<std::string> explode(const std::string& str, const char delimiter) {
	std::stringstream ss(str);
	std::string item;
	std::vector<std::string> result;
	while (std::getline(ss, item, delimiter))
		result.push_back(std::move(item));
	return result;
}

std::vector<std::string> explode(std::string str, const std::string& delimiterList,
								 bool skipEmpty)
{
	std::vector<std::string> result;
	size_t pos = 0;
	while (pos != std::string::npos) {
		pos = str.find_first_of(delimiterList);
		if (!(skipEmpty && pos == 0))
			result.emplace_back(str.substr(0, pos));
		str = str.substr(pos + 1);
	}
	return result;
}

std::string join(const std::vector<std::string>& strList, const std::string& delimiter) {
	std::string result;
	size_t size = strList.size();
	for (size_t i = 0; i < size; ++i) {
		result += strList[i];
		if (i + 1 < size)
			result += delimiter;
	}
	return result;
}

bool endsWith(const std::string& str, const std::string& ending) {
	if (ending.size() > str.size())
		return false;
	return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}

std::string xorEncrypt(const std::string& str, const std::string& key) {
	size_t strSize = str.size();
	size_t keySize = key.size() + 1;
	std::string result;
	for (size_t i = 0; i < strSize; ++i)
		result += str[i] ^ key[i % keySize];
	return result;
}


// XML
int xmlChildrenCount(const pugi::xml_node& node, const std::string& childName) {
	if (childName.empty())
		return (int)std::distance(node.children().begin(),
								  node.children().end());
	else
		return (int)std::distance(node.children(childName.c_str()).begin(),
								  node.children(childName.c_str()).end());
}

void xmlDeleteAllChildren(pugi::xml_node& node) {
	for (auto child = node.first_child(); child; ) {
		auto next = child.next_sibling();
		node.remove_child(child);
		child = next;
	}
}


// Other
void sleep(int time) {
	// Sleep(time);  // Windows
	// usleep(time * 1000);  // Unix
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

std::string getTime(const char* format) {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char time[30];
	strftime(time, sizeof(time), format, localtime(&now));
	return time;

	/*struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm *tm = localtime(&tv.tv_sec);
	char time[30];
	strftime(time, sizeof(time), format, tm);
	return time;*/

	/* // Time with milliseconds
	strftime(time, sizeof(time), "%d.%m.%Y %H:%M:%S:%%06u", tm);
	char buf[30];
	snprintf(buf, sizeof(buf), time, tv.tv_usec/1000);
	return buf; */
}

std::string intToHex(int number, size_t length) {
	std::string result(length, '0');
	for (size_t i = 0, j = (length-1)*4 ; i < length; ++i, j -= 4)
		result[i] = HEX_DATA[(number >> j) & 0x0f];
	return result;
}

char hexCharToDec(char c) {
	for (int i = 0; i < 16; ++i) {
		if (HEX_DATA[i] == c)
			return i;
	}
	return -1;
}

}  // End namespace
