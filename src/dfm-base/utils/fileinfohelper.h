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
#ifndef FILEINFOHELPER_H
#define FILEINFOHELPER_H
#include "dfm_base_global.h"
#include "utils/thumbnailprovider.h"

#include <dfm-io/core/dfileinfo.h>

#include <QObject>

namespace dfmbase {
class FileInfoHelper : public QObject
{
    Q_OBJECT
public:
    ~FileInfoHelper() override;
    static FileInfoHelper &instance();

private:
    explicit FileInfoHelper(QObject *parent = nullptr);

Q_SIGNALS:
    void createThumbnailFinished(const QUrl &sourceFile, const QString &thumbnailPath) const;
    void createThumbnailFailed(const QString &sourceFilePath) const;
    void mediaDataFinished(const QUrl &sourceFile, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties);
};
}

#endif   // FILEINFOHELPER_H
