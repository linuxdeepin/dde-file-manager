/**
 * @brief   Interface for file extensions
 * @package fileext
 * @file    fileext.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    12.07.2016 -- 10.02.2018
 */
#include <fstream>

#include "tools.hpp"

#include "fileext.hpp"
#include <iostream>

namespace fileext {

/** Config script file path */
const std::string LIB_PATH = tools::PROGRAM_PATH + "/files/libs";
const std::string SCRIPT_FILE = LIB_PATH + "/xpathconfig.min.js";

// public:
FileExtension::FileExtension(const std::string& fileName)
	: m_fileName(fileName) {}

}  // End namespace
