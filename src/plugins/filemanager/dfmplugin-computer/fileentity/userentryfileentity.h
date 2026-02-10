// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERENTRYFILEENTITY_H
#define USERENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmplugin_computer {

class UserEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
public:
    explicit UserEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order() const override;
    virtual QUrl targetUrl() const override;

private:
    QString dirName;
};

}
#endif   // USERENTRYFILEENTITY_H
