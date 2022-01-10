/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef SIDEBARSERVICE_H
#define SIDEBARSERVICE_H

#include "sidebar_defines.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

class SideBarServicePrivate;
class SideBarService final : public dpf::PluginService, dpf::AutoServiceRegister<SideBarService>
{
    Q_OBJECT
    Q_DISABLE_COPY(SideBarService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.SideBarService";
    }

    void addItem(const SideBar::ItemInfo &info, const SideBar::CdActionCallback &cdFunc,
                 const SideBar::ContextMenuCallback &menuFunc, const SideBar::RenameCallback &renameFunc = nullptr);
    void removeItem(const QUrl &url);
    void updateItem(const QUrl &url, const QString &newName);

private:
    explicit SideBarService(QObject *parent = nullptr);
    virtual ~SideBarService() override;

    QScopedPointer<SideBarServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // SIDEBARSERVICE_H
