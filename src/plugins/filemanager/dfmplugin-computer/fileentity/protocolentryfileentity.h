// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCOLENTRYFILEENTITY_H
#define PROTOCOLENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmplugin_computer {

class ProtocolEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT
public:
    explicit ProtocolEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QString editDisplayText() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order() const override;
    virtual quint64 sizeTotal() const override;
    virtual quint64 sizeUsage() const override;
    virtual void refresh() override;
    virtual QUrl targetUrl() const override;
    virtual bool renamable() const override;
};

}
#endif   // PROTOCOLENTRYFILEENTITY_H
