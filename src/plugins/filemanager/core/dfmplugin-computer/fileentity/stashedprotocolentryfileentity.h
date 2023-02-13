// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STASHEDPROTOCOLENTRYFILEENTITY_H
#define STASHEDPROTOCOLENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/file/entry/entities/abstractentryfileentity.h"

namespace dfmplugin_computer {

class StashedProtocolEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
public:
    explicit StashedProtocolEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder order() const override;
    virtual bool isAccessable() const override;
};

}

#endif   // STASHEDPROTOCOLENTRYFILEENTITY_H
