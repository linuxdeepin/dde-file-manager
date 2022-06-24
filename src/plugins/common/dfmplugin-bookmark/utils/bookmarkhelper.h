/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef BOOKMARKHELPER_H
#define BOOKMARKHELPER_H

#include "dfmplugin_bookmark_global.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"

#include <QObject>

DPBOOKMARK_BEGIN_NAMESPACE

class BookMarkHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkHelper)

public:
    static BookMarkHelper *instance();
    QString scheme();
    QUrl rootUrl();
    QIcon icon();

    // services instance
    static DSB_FM_NAMESPACE::SideBarService *sideBarServIns();
    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServIns();
    static DSC_NAMESPACE::MenuService *menuServIns();

private:
    explicit BookMarkHelper(QObject *parent = nullptr);
};

DPBOOKMARK_END_NAMESPACE

#endif   // BOOKMARKHELPER_H
