// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
