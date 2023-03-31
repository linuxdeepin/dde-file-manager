// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTENTRYFILEENTITY_H
#define VAULTENTRYFILEENTITY_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/file/entry/entities/abstractentryfileentity.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/utils/filestatisticsjob.h>

namespace dfmplugin_vault {
class VaultEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT
public:
    explicit VaultEntryFileEntity(const QUrl &url);
    virtual ~VaultEntryFileEntity() override;

    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;

    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder order() const override;

    virtual void refresh() override;
    virtual quint64 sizeTotal() const override;
    virtual QUrl targetUrl() const override;

public slots:
    void slotFileDirSizeChange(qint64 size, int filesCount, int directoryCount);

    void slotFinishedThread();

private:
    qint64 vaultTotal { 0 };
    qint64 totalchange { 0 };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    mutable bool showSizeState { false };
};
}
#endif   // VAULTENTRYFILEENTITY_H
