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

#ifndef MASTEREDMEDIACONTROLLER_P_H
#define MASTEREDMEDIACONTROLLER_P_H

#include <QPointer>
#include "private/dabstractfilewatcher_p.h"
#include "masteredmediacontroller.h"
#include <ddiskmanager.h>


class DFMShadowedDirIterator : public DDirIterator
{
public:
    DFMShadowedDirIterator(const QUrl &path,
                           const QStringList &nameFilters,
                           QDir::Filters filter,
                           QDirIterator::IteratorFlags flags);

    DUrl next() override;

    bool hasNext() const override;

    QString fileName() const override;

    DUrl fileUrl() const override;

    const DAbstractFileInfoPointer fileInfo() const override;

    DUrl url() const override;

private:
    QSharedPointer<QDirIterator> iterator;
    QSharedPointer<QDirIterator> stagingiterator;
    QString mntpoint;
    QString devfile;
    QSet<QString> seen;
    QSet<DUrl> skip;
    DUrl changeScheme(DUrl in) const;
    DUrl changeSchemeUpdate(DUrl in);
};

class MasteredMediaFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit MasteredMediaFileWatcherPrivate(MasteredMediaFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;
    bool handleGhostSignal(const DUrl &target, DAbstractFileWatcher::SignalType1 signal, const DUrl &url) override;

    QPointer<DAbstractFileWatcher> proxyStaging;
    QPointer<DAbstractFileWatcher> proxyOnDisk;
    QScopedPointer<DDiskManager> diskm;

    Q_DECLARE_PUBLIC(MasteredMediaFileWatcher)
};


#endif // MASTEREDMEDIACONTROLLER_P_H
