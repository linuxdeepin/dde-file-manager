/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "menuservicehelper.h"

DSC_BEGIN_NAMESPACE

void MenuServiceHelper::templateMenu(QMenu *menu)
{
    // TODO(Lee):
    menu->addAction("template action");
}

void MenuServiceHelper::desktopFileMenu(QMenu *menu)
{
    // TODO(Lee):
    menu->addAction("oem action");
}

void MenuServiceHelper::extendCustomMenu(QMenu *menu,
                                         bool isNormal,
                                         const QUrl &dir,
                                         const QUrl &focusFile,
                                         const QList<QUrl> &selected)
{
    // TODO(Lee):
    Q_UNUSED(menu)
    Q_UNUSED(isNormal)
    Q_UNUSED(dir)
    Q_UNUSED(focusFile)
    Q_UNUSED(selected)
    menu->addAction("custom action one");
}

void MenuServiceHelper::extensionPluginCustomMenu(QMenu *menu,
                                                  bool isNormal,
                                                  const QUrl &currentUrl,
                                                  const QUrl &focusFile,
                                                  const QList<QUrl> &selected)
{
    // TODO(Lee):
    Q_UNUSED(menu)
    Q_UNUSED(isNormal)
    Q_UNUSED(currentUrl)
    Q_UNUSED(focusFile)
    Q_UNUSED(selected)
    menu->addAction("extension action one");
}

DSC_END_NAMESPACE
