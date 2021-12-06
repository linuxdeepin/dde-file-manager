/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "dfm-base/base/abstractfileinfo.h"
#include "dfm-base/dfm_base_global.h"

DFMBASE_BEGIN_NAMESPACE

class FileUtils
{
public:
    static bool mkdir(const QUrl &url, const QString &dirName, QString *errorString = nullptr);
    static bool touch(const QUrl &url, const QString &fileName, QString *errorString = nullptr);
    static QString formatSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList());

    static QMap<QString, QString> getKernelParameters();
};

DFMBASE_END_NAMESPACE

#endif   // FILEUTILS_H
