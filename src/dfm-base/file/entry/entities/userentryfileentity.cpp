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
#include "userentryfileentity.h"
#include "file/entry/entryfileinfo.h"
#include "base/standardpaths.h"

#include <QDebug>
#include <QMenu>

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class UserEntryFileEntity
 * \brief class that present local user directories
 */
UserEntryFileEntity::UserEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kUserDir)) {
        qWarning() << "wrong suffix in" << __FUNCTION__ << "url";
        abort();
    }

    dirName = url.path().remove("." + QString(SuffixInfo::kUserDir));
}

QString UserEntryFileEntity::displayName() const
{
    return StandardPaths::displayName(dirName);
}

QIcon UserEntryFileEntity::icon() const
{
    return QIcon::fromTheme(StandardPaths::iconName(dirName));
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

void UserEntryFileEntity::onOpen()
{
}

EntryFileInfo::EntryOrder UserEntryFileEntity::order() const
{
    return EntryFileInfo::EntryOrder::kOrderUserDir;
}

QUrl UserEntryFileEntity::targetUrl() const
{
    QString path = StandardPaths::location(dirName);
    if (path.isEmpty())
        return QUrl();
    QUrl targetUrl;
    targetUrl.setScheme(dfmbase::SchemeTypes::kFile);
    targetUrl.setPath(path);
    return targetUrl;
}

QMenu *UserEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();

    menu->addAction(ContextMenuActionTrs::trOpenInNewWin());
    menu->addAction(ContextMenuActionTrs::trOpenInNewTab());
    menu->addSeparator();

    menu->addAction(ContextMenuActionTrs::trProperties());
    return menu;
}

DFMBASE_END_NAMESPACE
