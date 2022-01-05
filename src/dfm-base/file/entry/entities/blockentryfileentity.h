/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef BLOCKENTRYFILEENTITY_H
#define BLOCKENTRYFILEENTITY_H

#include "dfm-base/dfm_base_global.h"
#include "abstractentryfileentity.h"

DFMBASE_BEGIN_NAMESPACE

class BlockEntryFileEntity : public AbstractEntryFileEntity
{
public:
    explicit BlockEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual void onOpen() override;
    virtual EntryFileInfo::EntryOrder order() const override;

    virtual long sizeTotal() const override;
    virtual long sizeUsage() const override;
    virtual QString fileSystem() const override;
    virtual void refresh() override;
    virtual bool removable() override;
    virtual QMenu *createMenu() override;
    virtual QUrl targetUrl() const override;
    virtual bool isEncrypted() const override;
    virtual bool isUnlocked() const override;
    virtual QString clearDeviceId() const override;

private:
    QString getNameOrAlias() const;
    QString getNameOfOptical() const;
    QString getIdLabel() const;
    QVariant getProperty(const char *const key) const;
    bool showSizeAndProgress() const;

private:
    QVariantMap datas;
};

DFMBASE_END_NAMESPACE

#endif   // BLOCKENTRYFILEENTITY_H
