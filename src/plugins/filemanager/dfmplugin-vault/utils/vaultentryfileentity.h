/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef VAULTENTRYFILEENTITY_H
#define VAULTENTRYFILEENTITY_H

#include "dfmplugin_vault_global.h"
#include "dfm-base/file/entry/entities/abstractentryfileentity.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/utils/filestatisticsjob.h"

DPVAULT_BEGIN_NAMESPACE
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
    virtual void onOpen() override;
    virtual DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder order() const override;

    virtual QMenu *createMenu() override;
    virtual void refresh() override;
    virtual quint64 sizeTotal() const override;
    virtual QUrl targetUrl() const override;

public slots:
    void slotFileDirSizeChange(qint64 size);

    void slotFinishedThread();

private:
    qint64 vaultTotal { 0 };
    qint64 totalchange { 0 };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    mutable bool showSizeState { false };
};
DPVAULT_END_NAMESPACE
#endif   // VAULTENTRYFILEENTITY_H
