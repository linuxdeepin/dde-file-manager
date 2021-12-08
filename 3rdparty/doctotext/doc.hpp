/**
 * @brief     DOC files into HTML сonverter
 * @package   doc
 * @file      doc.hpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright rembish (https://github.com/rembish/TextAtAnyCost)
 * @version   1.0
 * @date      03.08.2017 -- 18.10.2017
 */
#pragma once

#include <string>
#include <vector>

#include "pugixml.hpp"
#include "cfb.hpp"
#include "fileext.hpp"


/**
 * @namespace doc
 * @brief
 *     DOC files into HTML сonverter
 */
namespace doc {

/**
 * @class Doc
 * @brief
 *     DOC files into HTML сonverter
 */
class Doc: public fileext::FileExtension, public cfb::Cfb {
public:
	/**
	 * @param[in] fileName
	 *     File name
	 * @since 1.0
	 */
	Doc(const std::string& fileName);

	/** Destructor */
	virtual ~Doc() = default;

	/**
	 * @brief
	 *     Convert file to HTML-tree
	 * @param[in] addStyle
	 *     Should read and add styles to HTML-tree
	 * @param[in] extractImages
	 *     True if should extract images
	 * @param[in] mergingMode
	 *     Colspan/rowspan processing mode
	 * @since 1.0
	 */
    int convert(bool addStyle = true, bool extractImages = false, char mergingMode = 0) override;
};

}  // End namespace
