/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_optical {
class MasteredMediaFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET

public:
    explicit MasteredMediaFileInfo(const QUrl &url);

    bool exists() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isDir() const override;
    QString fileDisplayName() const override;
    QString fileCopyName() const override;
    QVariantHash extraProperties() const override;
    bool canRedirectionFileUrl() const override;
    QUrl redirectedFileUrl() const override;
    QUrl parentUrl() const override;
    bool canDrop() override;
    bool canRename() const override;
    void refresh() override;
    bool canDragCompress() const override;
    bool canHidden() const override;
    QString emptyDirectoryTip() const override;
    virtual Qt::DropActions supportedDropActions() override;

private:
    void backupInfo(const QUrl &url);

private:
    QUrl backerUrl;
    QString curDevId;
    QVariantMap devInfoMap;
};

}

#endif   // MASTEREDMEDIAFILEINFO_H
