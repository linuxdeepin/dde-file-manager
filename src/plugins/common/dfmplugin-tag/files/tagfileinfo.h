/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef TAGFILEINFO_H
#define TAGFILEINFO_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_tag {
class TagFileInfoPrivate;
class TagFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
    friend class TagFileInfoPrivate;
    TagFileInfoPrivate *d;

public:
    explicit TagFileInfo(const QUrl &url);
    ~TagFileInfo() override;

    bool exists() const override;
    QFile::Permissions permissions() const override;
    virtual bool isAttributes(const FileIsType type) const override;

    QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    FileType fileType() const override;
    QIcon fileIcon() override;

    QString localFilePath() const;
    QString tagName() const;
};

using TagFileInfoPointer = QSharedPointer<TagFileInfo>;

}

#endif   // TAGFILEINFO_H
