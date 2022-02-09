/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef RECENTROOTFILEINFO_H
#define RECENTROOTFILEINFO_H

#include "dfmplugin_recent_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

DFMBASE_USE_NAMESPACE
DPRECENT_BEGIN_NAMESPACE

class RecentFileInfoPrivate;
class RecentFileInfo : public AbstractFileInfo
{
    Q_GADGET
    friend class RecentFileInfoPrivate;

public:
    explicit RecentFileInfo(const QUrl &url);
    ~RecentFileInfo() override;

    virtual bool exists() const override;
    virtual QFile::Permissions permissions() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;

private:
    RecentFileInfoPrivate *d;
};

using RecentFileInfoPointer = QSharedPointer<RecentFileInfo>;
DPRECENT_END_NAMESPACE

#endif   // RECENTROOTFILEINFO_H
