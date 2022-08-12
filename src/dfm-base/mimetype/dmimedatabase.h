/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#ifndef DMIMEDATABASE_H
#define DMIMEDATABASE_H

#include "dfm_base_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QMimeDatabase>

namespace dfmbase {

class DMimeDatabase : public QMimeDatabase
{
    Q_DISABLE_COPY(DMimeDatabase)

public:
    DMimeDatabase();

    QMimeType mimeTypeForFile(const QUrl &url, MatchMode mode = MatchDefault) const;
    QMimeType mimeTypeForFile(const AbstractFileInfoPointer &fileInfo, MatchMode mode = MatchDefault) const;

    QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode, const QString &inod, const bool isGvfs = false) const;

    QMimeType mimeTypeForUrl(const QUrl &url) const;
private:
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode, const QString &inod, const bool isGvfs = false) const;


private:
    QHash<QString, QMimeType> inodMimetypeCache;
};

}

#endif   // DMIMEDATABASE_H
