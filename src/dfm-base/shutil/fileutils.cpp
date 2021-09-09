/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "shutil/fileutils.h"
#include "shutil/mimedatabase.h"

#include <QFileInfo>

DFMBASE_BEGIN_NAMESPACE

/*!
 * @class FileUtils
 *
 * @brief Utility class providing static helper methods for file management
 */

/*!
 * \brief FileUtils::isArchive 判断是否是归档包
 *
 * \param path 文件的路径
 *
 * \return bool 是否是归档包
 */
bool FileUtils::isArchive(const QString &path)
{
    QFileInfo f(path);
    if (f.exists()) {
        return MimeDatabase::supportMimeFileType(MimeDatabase::Archives)
                .contains(MimeDatabase::mimeTypeForFile(f).name());
    } else {
        return false;
    }
}

DFMBASE_END_NAMESPACE
