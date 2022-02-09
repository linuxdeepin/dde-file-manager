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
#ifndef PROTOCOLENTRYFILEENTITY_H
#define PROTOCOLENTRYFILEENTITY_H

#include "dfm-base/dfm_base_global.h"
#include "abstractentryfileentity.h"

DFMBASE_BEGIN_NAMESPACE

class ProtocolEntryFileEntity : public AbstractEntryFileEntity
{
public:
    explicit ProtocolEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual void onOpen() override;
    virtual EntryFileInfo::EntryOrder order() const override;
    virtual qint64 sizeTotal() const override;
    virtual qint64 sizeUsage() const override;
    virtual void refresh() override;
    virtual QUrl targetUrl() const override;
    virtual QMenu *createMenu() override;
};

DFMBASE_END_NAMESPACE
#endif   // PROTOCOLENTRYFILEENTITY_H
