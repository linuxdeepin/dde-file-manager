// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTENTRYFILEENTITY_H
#define VAULTENTRYFILEENTITY_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/utils/filestatisticsjob.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>

// NOTE: the namespaceand classname cannot rename,
// it's as a reflectionb object
namespace dfmplugin_vault {
class VaultEntryFileEntity : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit VaultEntryFileEntity(QObject *parent = nullptr);
    Q_INVOKABLE ~VaultEntryFileEntity() override;

    Q_INVOKABLE QString displayName() const;
    Q_INVOKABLE QIcon icon() const;
    Q_INVOKABLE bool exists() const;

    Q_INVOKABLE bool showProgress() const;
    Q_INVOKABLE bool showTotalSize() const;
    Q_INVOKABLE bool showUsageSize() const;
    Q_INVOKABLE DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE quint64 sizeTotal() const;
    Q_INVOKABLE QUrl targetUrl() const;

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
