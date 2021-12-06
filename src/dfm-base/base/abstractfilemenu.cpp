/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "abstractfilemenu.h"

#include <QAction>
#include <QMenu>

DFMBASE_BEGIN_NAMESPACE

namespace FileMenuTypes {
const QString kCreateNewDir { QObject::tr("Create New Dir") };
const QString kCreateNewDoc { QObject::tr("Create New Doc") };
const QString kViewModeSwitch { QObject::tr("Switch View Mode") };
const QString kViewSortSwitch { QObject::tr("Switch View Sort") };
const QString kOpenAsAdmin { QObject::tr("Open As Admin") };
const QString kOpenInTerminal { QObject::tr("Open In Terminal") };
const QString kSelectAll { QObject::tr("Select All") };
const QString kProperty { QObject::tr("Property") };
const QString kCut { QObject::tr("Cut") };
const QString kCopy { QObject::tr("Copy") };
const QString kPaste { QObject::tr("Paste") };
const QString kRename { QObject::tr("Rename") };
}   // namespace FileMenuTypes

AbstractFileMenu::AbstractFileMenu(QObject *parent)
    : QObject(parent)
{
}

AbstractFileMenu::~AbstractFileMenu()
{
}

QMenu *AbstractFileMenu::build(AbstractFileMenu::MenuMode mode,
                               const QUrl &rootUrl,
                               const QUrl &foucsUrl,
                               const QList<QUrl> &selected)
{
    Q_UNUSED(rootUrl)
    Q_UNUSED(foucsUrl)
    Q_UNUSED(selected)

    if (AbstractFileMenu::MenuMode::kEmpty == mode) {
        QMenu *menu = new QMenu(nullptr);
        menu->addAction(FileMenuTypes::kCreateNewDir);
        menu->addAction(FileMenuTypes::kCreateNewDoc);
        menu->addAction(FileMenuTypes::kViewModeSwitch);
        menu->addAction(FileMenuTypes::kViewSortSwitch);
        menu->addAction(FileMenuTypes::kOpenAsAdmin);
        menu->addAction(FileMenuTypes::kOpenInTerminal);
        menu->addAction(FileMenuTypes::kSelectAll);
        menu->addAction(FileMenuTypes::kProperty);
        return menu;
    }

    if (AbstractFileMenu::MenuMode::kNormal == mode) {
        QMenu *menu = new QMenu(nullptr);
        menu->addAction(FileMenuTypes::kCreateNewDir);
        menu->addAction(FileMenuTypes::kCreateNewDoc);
        menu->addAction(FileMenuTypes::kViewModeSwitch);
        menu->addAction(FileMenuTypes::kViewSortSwitch);
        menu->addAction(FileMenuTypes::kOpenAsAdmin);
        menu->addAction(FileMenuTypes::kOpenInTerminal);
        menu->addAction(FileMenuTypes::kSelectAll);
        menu->addAction(FileMenuTypes::kProperty);
        return menu;
    }

    return nullptr;
}

DFMBASE_END_NAMESPACE
