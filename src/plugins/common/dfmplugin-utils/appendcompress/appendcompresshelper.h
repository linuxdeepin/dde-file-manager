/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef APPENDCOMPRESSHELPER_H
#define APPENDCOMPRESSHELPER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_utils {

class AppendCompressHelper : public QObject
{
    Q_OBJECT
public:
    static bool setMouseStyle(const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction);
    static bool dragDropCompress(const QUrl &toUrl, const QList<QUrl> &fromUrls);
    static bool isCompressedFile(const QUrl &toUrl);

private:
    explicit AppendCompressHelper(QObject *parent = nullptr);
    static bool appendCompress(const QString &toFilePath, const QStringList &fromFilePaths);
    static bool canAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl);
};

}

#endif   // APPENDCOMPRESSHELPER_H
