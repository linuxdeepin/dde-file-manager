/*
* Copyright (C) 2019 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:      Xiao Zhiguo <xiaozhiguo@uniontech.com>
* Maintainer:  Xiao Zhiguo <xiaozhiguo@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCHEMEPLUGININTERFACE_H
#define SCHEMEPLUGININTERFACE_H

#include "dfmbaseview.h"
#include "dfmsidebariteminterface.h"
#include "dfmsidebaritem.h"

#include "dfmcrumbinterface.h"
#include "dfmcrumbmanager.h"

#define DFMFileSchemePluginInterface_iid "com.deepin.filemanager.DFMFileSchemePluginInterface_iid"

typedef QPair<QString, std::function<dde_file_manager::DFMCrumbInterface*()>> CrumbCreaterFunc;
typedef QPair<QString, std::function<dde_file_manager::DFMBaseView*()>> ViewCreatorFunc;
typedef QPair<QString, std::function<dde_file_manager::DFMSideBarItemInterface*()>> SideBarInterfaceCreaterFunc;

class SchemePluginInterface
{
public:
    virtual ~SchemePluginInterface() = default;

    // 创建SiseBarItem对应的右侧BaseView的回调函数
    virtual ViewCreatorFunc createViewTypeFunc() = 0;

    // 初始化SideBarItem，添加自定义的图标，文字，URL等构造信息
    virtual dde_file_manager::DFMSideBarItem *createSideBarItem() = 0;

    // 初始化SideBarItem对应的Handler，可重载右键菜单功能
    virtual SideBarInterfaceCreaterFunc createSideBarInterfaceTypeFunc() = 0;

    // 初始化面包屑控制器CrumbControl
    virtual CrumbCreaterFunc createCrumbCreaterTypeFunc() = 0;

    // 返回插件的名称，用于判断插件的类型，与SideBarItem、DFMBaseView、CrumbControl关联起来
    // [protocol]://[host]:[port]/[path]?[query]#[hash] 对应于QUrl::host()返回的Url
    virtual QString pluginName() = 0;

    // 查询插件SideBarItem是否支持拖拽
    virtual bool isSideBarItemSupportedDrop() = 0;
    // 向SideBarItem拖拽文件调用DBus接口
    virtual bool dropFileToPlugin(QList<DUrl> srcUrls, DUrl desUrl) = 0;
};

Q_DECLARE_INTERFACE(SchemePluginInterface, DFMFileSchemePluginInterface_iid)

#endif // SCHEMEPLUGININTERFACE_H
