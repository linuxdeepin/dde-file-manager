/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "smbsharefilemenu.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/device/devicecontroller.h"

#include <QMenu>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define CreateContainerByType(type) ActionTypeManager::instance().actionDataContainerByType(type)

SmbShareFileMenu::SmbShareFileMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *SmbShareFileMenu::build(QWidget *parent, AbstractMenu::MenuMode mode, const QUrl &rootUrl, const QUrl &focusUrl, const QList<QUrl> &selected, QVariant customData)
{
    Q_UNUSED(rootUrl)
    Q_UNUSED(focusUrl)
    Q_UNUSED(customData)

    QMenu *menu = new QMenu(parent);

    if (mode == AbstractMenu::MenuMode::kEmpty)
        return menu;
    if (selected.count() != 1)
        return menu;

    QVector<ActionDataContainer> acts;
    acts.append(CreateContainerByType(kActOpen));
    acts.append(CreateContainerByType(kActSeparator));
    acts.append(CreateContainerByType(kActOpenInNewWindow));

    delete menu;
    menu = createMenuByContainer(acts);

    winId = DSB_FM_NAMESPACE::WindowsService::service()->findWindowId(parent);
    selectedUrl = selected.first();

    return menu;
}

void SmbShareFileMenu::actionBusiness(QAction *act)
{
    int type = act->data().toInt();
    if (type != kActOpen && type != kActOpenInNewWindow)
        return;

    DeviceController::instance()->mountNetworkDevice(selectedUrl.toString(), [this, type](bool ok, dfmmount::DeviceError err, const QString &mntPath) {
        if (!ok && err != DFMMOUNT::DeviceError::GIOErrorAlreadyMounted) {
            DialogManagerInstance->showErrorDialogWhenMountDeviceFailed(err);
        } else {
            QUrl u = QUrl::fromLocalFile(mntPath);
            if (type == kActOpen)
                dpfInstance.eventDispatcher().publish(GlobalEventType::kChangeCurrentUrl, winId, u);
            else
                dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenNewWindow, u);
        }
    });
}

QMenu *SmbShareFileMenu::createMenuByContainer(const QVector<ActionDataContainer> &containers, QWidget *parent)
{
    QMenu *menu = new QMenu(parent);

    for (auto c : containers) {
        if (c.actionType() == kActSeparator) {
            menu->addSeparator();
            continue;
        }

        QAction *act = new QAction(c.name(), menu);
        act->setData(c.actionType());
        menu->addAction(act);

        if (c.childrenActionsData().count() > 0) {
            QMenu *subMenu = new QMenu(menu);
            for (auto cc : c.childrenActionsData()) {
                QAction *subAct = new QAction(cc.name(), subMenu);
                subAct->setData(cc.actionType());
                subMenu->addAction(subAct);
            }
            act->setMenu(subMenu);
        }
    }

    return menu;
}
