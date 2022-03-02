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
#include "bookmarkhelper.h"

DPBOOKMARK_USE_NAMESPACE

BookMarkHelper *BookMarkHelper::instance()
{
    static BookMarkHelper instance;
    return &instance;
}

QString BookMarkHelper::scheme()
{
    return "bookmark";
}

QUrl BookMarkHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

QIcon BookMarkHelper::icon()
{
    return QIcon::fromTheme("folder-bookmark-symbolic");
}

dfm_service_filemanager::WindowsService *BookMarkHelper::winServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WindowsService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
}

dfm_service_filemanager::SideBarService *BookMarkHelper::sideBarServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

dfm_service_filemanager::WorkspaceService *BookMarkHelper::workspaceServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WorkspaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WorkspaceService>(DSB_FM_NAMESPACE::WorkspaceService::name());
}

dfm_service_common::MenuService *BookMarkHelper::menuServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::MenuService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::MenuService>(DSC_NAMESPACE::MenuService::name());
}

BookMarkHelper::BookMarkHelper(QObject *parent) : QObject(parent) {}
