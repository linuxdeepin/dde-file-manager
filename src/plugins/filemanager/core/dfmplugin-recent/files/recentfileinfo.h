/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef RECENTFILEINFO_H
#define RECENTFILEINFO_H

#include "dfmplugin_recent_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

namespace dfmplugin_recent {
class RecentFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
public:
    explicit RecentFileInfo(const QUrl &url);
    ~RecentFileInfo() override;

    virtual bool exists() const override;
    virtual QFile::Permissions permissions() const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;

    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;

    virtual QVariant customData(int role) const override;
};

using RecentFileInfoPointer = QSharedPointer<RecentFileInfo>;
}

#endif   // RECENTFILEINFO_H
