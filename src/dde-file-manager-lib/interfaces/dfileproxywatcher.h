/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DFILEPROXYWATCHER_H
#define DFILEPROXYWATCHER_H

#include "dabstractfilewatcher.h"

#include <functional>

class DFileProxyWatcherPrivate;
class DFileProxyWatcher : public DAbstractFileWatcher
{
public:
    explicit DFileProxyWatcher(const DUrl &url, DAbstractFileWatcher *proxy,
                               std::function<DUrl (const DUrl &)> urlConvertFun,
                               QObject *parent = 0);

private slots:
    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void onSubfileCreated(const DUrl &url);

    /**
     * @brief onFileModified 文件修改
     * @param url 文件url
     */
    void onFileModified(const DUrl &url);

private:
    Q_DECLARE_PRIVATE(DFileProxyWatcher)
};

#endif // DFILEPROXYWATCHER_H
