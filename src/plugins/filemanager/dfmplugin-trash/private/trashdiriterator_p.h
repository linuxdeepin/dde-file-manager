// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHDIRITERATORPRIVATE_H
#define TRASHDIRITERATORPRIVATE_H

#include "dfmplugin_trash_global.h"
#include <dfm-base/interfaces/abstractdiriterator.h>

#include <dfm-io/denumerator.h>

#include <QDirIterator>

namespace dfmplugin_trash {

class TrashDirIterator;

class TrashDirIteratorPrivate
{
    friend class TrashDirIterator;

public:
    TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                            DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                            TrashDirIterator *qq);
    ~TrashDirIteratorPrivate();

private:
    TrashDirIterator *q { nullptr };
    QSharedPointer<DFMIO::DEnumerator> dEnumerator = nullptr;
    QUrl currentUrl;
    QMap<QString, QString> fstabMap;
    FileInfoPointer fileInfo{nullptr};
    std::atomic_bool once{ false };
};

}
#endif   // TRASHDIRITERATORPRIVATE_H
