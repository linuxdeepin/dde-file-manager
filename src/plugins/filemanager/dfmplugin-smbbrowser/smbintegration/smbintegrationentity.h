// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STASHEDSMBINTEGRATIONENTITY_H
#define STASHEDSMBINTEGRATIONENTITY_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-base/file/entry/entities/abstractentryfileentity.h"

namespace dfmplugin_smbbrowser {

class SmbIntegrationEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
public:
    explicit SmbIntegrationEntity(const QUrl &url);

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

#endif   // STASHEDSMBINTEGRATIONENTITY_H
