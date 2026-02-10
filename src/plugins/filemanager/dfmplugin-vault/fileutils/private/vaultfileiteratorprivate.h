// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEDIRITERATORPTIVATE_H
#define VAULTFILEDIRITERATORPTIVATE_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <QDirIterator>
#include <QDebug>

#include <dfm-io/denumerator.h>

namespace dfmplugin_vault {
class VaultFileIterator;
class VaultFileIteratorPrivate : public QObject
{
    Q_OBJECT
    friend class VaultFileIterator;

public:
    explicit VaultFileIteratorPrivate(const QUrl &url,
                                      const QStringList &nameFilters,
                                      QDir::Filters filters,
                                      QDirIterator::IteratorFlags flags,
                                      VaultFileIterator *qp);

    ~VaultFileIteratorPrivate();

private:
    QSharedPointer<dfmio::DEnumerator> dfmioDirIterator = nullptr;   // dfmio的文件迭代器
    QUrl currentUrl;   // 当前迭代器所在位置文件的url
    QDir::Filters curFilters;   // 文件的当前的过滤flags
    bool isCurrent = false;   // 用来判断当前是否使用了一次next
    VaultFileIterator *const q;
};

}

#endif   //VAULTFILEDIRITERATORPTIVATE_H
