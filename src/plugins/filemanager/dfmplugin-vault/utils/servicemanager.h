/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H
#include "dfmplugin_vault_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/fileencrypt/fileencryptservice.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QObject>
namespace dfmplugin_vault {
class ServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceManager(QObject *parent = nullptr);

public:
    static DSB_FM_NAMESPACE::FileEncryptService *fileEncryptServiceInstance();

    static DSB_FM_NAMESPACE::SideBarService *sideBarServiceInstance();

    static DSB_FM_NAMESPACE::TitleBarService *titleBarServiceInstance();

    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServiceInstance();

    static DSC_NAMESPACE::FileOperationsService *fileOperationsServIns();

    static QMap<DSC_NAMESPACE::CPY_NAMESPACE::BasicExpandType, DSC_NAMESPACE::CPY_NAMESPACE::BasicExpand> basicViewFieldFunc(const QUrl &url);
};
}
#endif   // SERVICEMANAGER_H
