// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEDIRITERATOR_H
#define VAULTFILEDIRITERATOR_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/file/local/localdiriterator.h>

namespace dfmplugin_vault {
class VaultFileIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class VaultFileIteratorPrivate;

public:
    explicit VaultFileIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               QDir::Filters filters = QDir::NoFilter,
                               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    virtual ~VaultFileIterator() override;

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual QString fileName() const override;

    virtual QUrl fileUrl() const override;

    virtual const FileInfoPointer fileInfo() const override;

    virtual QUrl url() const override;

    bool initIterator() override;

private:
    QSharedPointer<dfmio::DEnumerator> dfmioDirIterator { Q_NULLPTR };
    QUrl currentUrl;
    QSet<QString> hideFileList;
};
}
#endif   // VAULTFILEDIRITERATOR_H
