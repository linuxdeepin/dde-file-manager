/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef MERGEDDESKTOPCONTROLLER_P_H
#define MERGEDDESKTOPCONTROLLER_P_H

#include "private/dabstractfilewatcher_p.h"

class MergedDesktopWatcherPrivate;
class MergedDesktopWatcher : public DAbstractFileWatcher
{
public:
    explicit MergedDesktopWatcher(const DUrl &url, DAbstractFileWatcher *baseWatcher, QObject *parent = nullptr);

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const DUrl &url);
    void removeWatcher(const DUrl &url);

    void onFileAttributeChanged(const DUrl &url);
    void onFileModified(const DUrl &url);

    Q_DECLARE_PRIVATE(MergedDesktopWatcher)
};

class MergedDesktopWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit MergedDesktopWatcherPrivate(DAbstractFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override
    {
        started = true;

        return true;
    }

    bool stop() override
    {
        started = false;

        return true;
    }

    QMap<DUrl, DAbstractFileWatcher *> urlToWatcherMap;

    Q_DECLARE_PUBLIC(MergedDesktopWatcher)
};


#endif // MERGEDDESKTOPCONTROLLER_P_H
