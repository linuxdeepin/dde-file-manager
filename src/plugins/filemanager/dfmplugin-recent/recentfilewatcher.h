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
#ifndef LOCALFILEWATCHER_H
#define LOCALFILEWATCHER_H

#include "dfmplugin_recent_global.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

class QUrl;

DFMBASE_USE_NAMESPACE
DPRECENT_BEGIN_NAMESPACE

class RecentFileWatcherPrivate;
class RecentFileWatcher : public AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit RecentFileWatcher() = delete;
    explicit RecentFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~RecentFileWatcher() override;
    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const QUrl &url);
    void removeWatcher(const QUrl &url);

    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);

    RecentFileWatcherPrivate *dptr;
};
DPRECENT_END_NAMESPACE

#endif   // LOCALFILEWATCHER_H
