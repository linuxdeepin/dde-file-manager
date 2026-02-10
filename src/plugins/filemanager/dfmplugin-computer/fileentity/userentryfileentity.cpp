// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userentryfileentity.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDebug>

using namespace dfmplugin_computer;

/*!
 * \class UserEntryFileEntity
 * \brief class that present local user directories
 */
UserEntryFileEntity::UserEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kUserDir)) {
        fmCritical() << "Invalid user directory URL suffix:" << url;
        abort();
    }

    dirName = url.path().remove("." + QString(SuffixInfo::kUserDir));
}

QString UserEntryFileEntity::displayName() const
{
    return DFMBASE_NAMESPACE::StandardPaths::displayName(dirName);
}

QIcon UserEntryFileEntity::icon() const
{
    return QIcon::fromTheme(DFMBASE_NAMESPACE::StandardPaths::iconName(dirName));
}

bool UserEntryFileEntity::exists() const
{
    return true;
}

bool UserEntryFileEntity::showProgress() const
{
    return false;
}

bool UserEntryFileEntity::showTotalSize() const
{
    return false;
}

bool UserEntryFileEntity::showUsageSize() const
{
    return false;
}

DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder UserEntryFileEntity::order() const
{
    return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderUserDir;
}

QUrl UserEntryFileEntity::targetUrl() const
{
    QString path = DFMBASE_NAMESPACE::StandardPaths::location(dirName);
    if (path.isEmpty()) {
        fmWarning() << "No path found for user directory:" << dirName;
        return QUrl();
    }

    QUrl targetUrl;
    targetUrl.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    targetUrl.setPath(path);
    return targetUrl;
}
