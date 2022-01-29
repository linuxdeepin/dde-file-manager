/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *
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
#ifndef TRASHFILEWATCHER_H
#define TRASHFILEWATCHER_H

#include "dfmplugin_trash_global.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

DPTRASH_BEGIN_NAMESPACE

class TrashFileWatcherPrivate;
class TrashFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    friend TrashFileWatcherPrivate;
    Q_OBJECT

public:
    explicit TrashFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~TrashFileWatcher() override;

private slots:
    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);
    void onSubfileCreated(const QUrl &url);

private:
    TrashFileWatcher() = delete;
    TrashFileWatcherPrivate *dptr;
};

DPTRASH_END_NAMESPACE

#endif   // TRASHFILEWATCHER_H
