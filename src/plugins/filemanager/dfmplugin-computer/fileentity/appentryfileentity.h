// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPENTRYFILEENTITY_H
#define APPENTRYFILEENTITY_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmbase {
class DesktopFile;
}

namespace dfmplugin_computer {

namespace ExtraPropertyName {
inline constexpr char kExecuteCommand[] { "execute_command" };
}   // namespace ExtraPropertyName

class AppEntryFileEntity : public DFMBASE_NAMESPACE::AbstractEntryFileEntity
{
    Q_OBJECT
public:
    explicit AppEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual QString description() const override;
    virtual DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order() const override;
    virtual QVariantHash extraProperties() const override;
    virtual bool isAccessable() const override;

private:
    QString getFormattedExecCommand() const;

private:
    QSharedPointer<DFMBASE_NAMESPACE::DesktopFile> desktopInfo;
    QUrl fileUrl;
};

}
#endif   // APPENTRYFILEENTITY_H
