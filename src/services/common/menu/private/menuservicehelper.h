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
#ifndef MENUSERVICEHELPER_H
#define MENUSERVICEHELPER_H

#include "dfm_common_service_global.h"

#include <QMenu>

DSC_BEGIN_NAMESPACE

class MenuServiceHelper
{
    friend class MenuServer;

public:
    static void templateMenu(QMenu *menu);

    static void desktopFileMenu(QMenu *menu);

    static void extendCustomMenu(QMenu *menu, bool isNormal,
                                 const QUrl &dir,
                                 const QUrl &focusFile,
                                 const QList<QUrl> &selected = {},
                                 bool onDesktop = false);

    static void extensionPluginCustomMenu(QMenu *menu, bool isNormal,
                                          const QUrl &currentUrl,
                                          const QUrl &focusFile,
                                          const QList<QUrl> &selected = {});
};

DSC_END_NAMESPACE
#endif   // MENUSERVICEHELPER_H
