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
#include "appentryfileentity.h"
#include "utils/computerutils.h"

#include "dfm-base/utils/desktopfile.h"

#include <QFile>
#include <QMenu>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_BEGIN_NAMESPACE

namespace ExtraPropertyName {
const char *const kExecuteCommand { "execute_command" };
}   // namespace ExtraPropertyName

/*!
 * \class AppEntryFileEntity
 * \brief class that present application entries.
 */
AppEntryFileEntity::AppEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    fileUrl = ComputerUtils::getAppEntryFileUrl(url);
    desktopInfo.reset(new DesktopFile(fileUrl.path()));
}

QString AppEntryFileEntity::displayName() const
{
    return desktopInfo->getDisplayName();
}

QIcon AppEntryFileEntity::icon() const
{
    return QIcon::fromTheme(desktopInfo->getIcon());
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

void AppEntryFileEntity::onOpen()
{
}

QString AppEntryFileEntity::description() const
{
    return tr("Double click to open it");
}

EntryFileInfo::EntryOrder AppEntryFileEntity::order() const
{
    return EntryFileInfo::EntryOrder::kOrderApps;
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

QMenu *AppEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();
    menu->addAction(ContextMenuActionTrs::trOpen());
    return menu;
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
    auto cmd = desktopInfo->getExec();
    for (const auto &param : unsupportedParams)
        cmd.remove(param);
    return cmd.remove("\"").remove("'");
}

DPCOMPUTER_END_NAMESPACE

QString ContextMenuActionTrs::trOpen()
{
    return DPCOMPUTER_NAMESPACE::AppEntryFileEntity::tr("Open");
}
