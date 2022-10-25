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
#ifndef MASTEREDMEDIAFILEWATCHER_H
#define MASTEREDMEDIAFILEWATCHER_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractfilewatcher.h"

namespace dfmplugin_optical {

class MasteredMediaFileWatcherPrivate;
class MasteredMediaFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    friend MasteredMediaFileWatcherPrivate;
    Q_OBJECT

public:
    explicit MasteredMediaFileWatcher(const QUrl &url, QObject *parent = nullptr);

private slots:
    void onFileDeleted(const QUrl &url);
    void onMountPointDeleted(const QString &id);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRename(const QUrl &fromUrl, const QUrl &toUrl);
    void onSubfileCreated(const QUrl &url);

private:
    MasteredMediaFileWatcherPrivate *dptr;
};

}

#endif   // MASTEREDMEDIAFILEWATCHER_H
