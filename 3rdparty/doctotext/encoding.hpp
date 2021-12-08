/**
 * @brief   Encoding/decoding functions
 * @package encoding
 * @file    encoding.hpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @version 1.0.1
 * @date    22.08.2017 -- 29.10.2017
 */
#pragma once

#include <string>


/**
 * @namespace encoding
 * @brief
 *     Encoding/decoding functions
 */
namespace encoding {
	/**
	 * @brief
	 *     Change string encoding
	 * @param[in] str
	 *     Input string
	 * @param[in] fromCode
	 *     Old encoding
	 * @param[in] toCode
	 *     New encoding
	 * @return
	 *     String in new encoding
	 * @note
	 *     Requires `iconv` library
	 * @since 1.0
	 */
	std::string decode(const std::string& str, const std::string& fromCode,
					   const std::string& toCode = "UTF-8");

	/**
	 * @brief
	 *     Decode HTML special entities
	 * @param[in] code
	 *     Input symbol code
	 * @param[in] base
	 *     Code number base
	 * @return
	 *     Decoded symbol
	 * @since 1.0
	 */
	std::string htmlSpecialDecode(const std::string& code, int base = 16);

}  // End namespace
