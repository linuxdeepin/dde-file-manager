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
#include "servicemanager.h"

#include <mutex>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
{
}

FileEncryptService *ServiceManager::fileEncryptServiceInstance()
{
    static FileEncryptService *vaultService = nullptr;
    if (vaultService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(FileEncryptService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        vaultService = ctx.service<FileEncryptService>(FileEncryptService::name());
        if (!vaultService) {
            qCritical() << "Failed, init vault \"sideBarService\" is empty";
            abort();
        }
    }
    return vaultService;
}

SideBarService *ServiceManager::sideBarServiceInstance()
{
    static SideBarService *sideBarService = nullptr;
    if (sideBarService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(SideBarService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        sideBarService = ctx.service<SideBarService>(SideBarService::name());
        if (!sideBarService) {
            qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
            abort();
        }
    }
    return sideBarService;
}

WindowsService *ServiceManager::windowServiceInstance()
{
    static WindowsService *windowsService = nullptr;
    if (windowsService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(WindowsService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        windowsService = ctx.service<WindowsService>(WindowsService::name());
        if (!windowsService) {
            qCritical() << "Failed, init windows \"sideBarService\" is empty";
            abort();
        }
    }
    return windowsService;
}

TitleBarService *ServiceManager::titleBarServiceInstance()
{
    static TitleBarService *titleBarService = nullptr;
    if (titleBarService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(TitleBarService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        titleBarService = ctx.service<TitleBarService>(TitleBarService::name());
        if (!titleBarService) {
            qCritical() << "Failed, init titlebar \"titleBarService\" is empty";
            abort();
        }
    }
    return titleBarService;
}

WorkspaceService *ServiceManager::workspaceServiceInstance()
{
    static WorkspaceService *workspaceService = nullptr;
    if (workspaceService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(WorkspaceService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
        if (!workspaceService) {
            qCritical() << "Failed, init workspace \"workspaceService\" is empty";
            abort();
        }
    }
    return workspaceService;
}

FileOperationsService *ServiceManager::fileOperationsServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
}

QMap<Property::BasicExpandType, Property::BasicExpand> ServiceManager::basicViewFieldFunc(const QUrl &url)
{
    Property::BasicExpand expandFiledMap;

    expandFiledMap.insert(Property::BasicFieldExpandEnum::kFilePosition, qMakePair(tr("Location"), url.url()));

    QMap<Property::BasicExpandType, Property::BasicExpand> expandMap;
    expandMap.insert(Property::BasicExpandType::kFieldReplace, expandFiledMap);
    return expandMap;
}
