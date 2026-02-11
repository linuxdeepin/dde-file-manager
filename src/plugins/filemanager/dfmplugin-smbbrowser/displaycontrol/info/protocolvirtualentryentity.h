// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCOLVIRTUALENTRYENTITY_H
#define PROTOCOLVIRTUALENTRYENTITY_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

DPSMBBROWSER_BEGIN_NAMESPACE

inline constexpr char kVEntrySuffix[] { "ventry" };
inline constexpr char kComputerProtocolSuffix[] { "protodev" };

class ProtocolVirtualEntryEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT

public:
    explicit ProtocolVirtualEntryEntity(const QUrl &url);

    // AbstractEntryFileEntity interface
    virtual QString displayName() const override;
    virtual QString editDisplayText() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual dfmbase::AbstractEntryFileEntity::EntryOrder order() const override;
    virtual QUrl targetUrl() const override;
    virtual bool renamable() const override;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // PROTOCOLVIRTUALENTRYENTITY_H
