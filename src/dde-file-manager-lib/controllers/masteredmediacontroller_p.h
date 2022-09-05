// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
