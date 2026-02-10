// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appentryfileentity.h"
#include "utils/computerutils.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/utils/desktopfile.h>

#include <QFile>

namespace dfmplugin_computer {

/*!
 * \class AppEntryFileEntity
 * \brief class that present application entries.
 */
AppEntryFileEntity::AppEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    fileUrl = ComputerUtils::getAppEntryFileUrl(url);
    desktopInfo.reset(new DFMBASE_NAMESPACE::DesktopFile(fileUrl.path()));
}

QString AppEntryFileEntity::displayName() const
{
    return desktopInfo->desktopDisplayName();
}

QIcon AppEntryFileEntity::icon() const
{
    return QIcon::fromTheme(desktopInfo->desktopIcon());
}

bool AppEntryFileEntity::exists() const
{
    return QFile(fileUrl.path()).exists();
}

bool AppEntryFileEntity::showProgress() const
{
    return false;
}

bool AppEntryFileEntity::showTotalSize() const
{
    return false;
}

bool AppEntryFileEntity::showUsageSize() const
{
    return false;
}

QString AppEntryFileEntity::description() const
{
    return tr("Double click to open it");
}

DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder AppEntryFileEntity::order() const
{
    return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderApps;
}

QVariantHash AppEntryFileEntity::extraProperties() const
{
    QVariantHash ret;
    ret.insert(ExtraPropertyName::kExecuteCommand, getFormattedExecCommand());
    return ret;
}

bool AppEntryFileEntity::isAccessable() const
{
    return exists();
}

QString AppEntryFileEntity::getFormattedExecCommand() const
{
    // no parameters transfered
    static const QStringList unsupportedParams {
        "%U",   // A list of Urls
        "%u",   // A Url
        "%F",   // A list of files
        "%f"   // A file
    };
    auto cmd = desktopInfo->desktopExec();
    for (const auto &param : unsupportedParams)
        cmd.remove(param);
    return cmd.remove("\"").remove("'");
}

}
