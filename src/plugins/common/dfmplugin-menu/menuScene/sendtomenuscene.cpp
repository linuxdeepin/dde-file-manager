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
#include "sendtomenuscene.h"
#include "private/sendtomenuscene_p.h"
#include "action_defines.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/bluetooth/bluetoothservice.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QFileInfo>
#include <QApplication>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

SendToMenuScene::SendToMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SendToMenuScenePrivate(this))
{
}

SendToMenuScene::~SendToMenuScene()
{
}

QString SendToMenuScene::name() const
{
    return SendToMenuCreator::name();
}

bool SendToMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    for (auto url : d->selectFiles) {
        auto f = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true);
        if (f->isDir()) {
            d->folderSelected = true;
            break;
        }
    }

    if (d->selectFiles.isEmpty())
        return false;

    return true;
}

bool SendToMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->isEmptyArea) {
        QMenu *sendToMenu = new QMenu(parent);
        d->addSubActions(sendToMenu);
        if (sendToMenu->actions().isEmpty()) {
            delete sendToMenu;
            sendToMenu = nullptr;
        } else {
            auto sendToAct = parent->addAction(d->predicateName[ActionID::kSendTo]);
            sendToAct->setMenu(sendToMenu);
            sendToAct->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, ActionID::kSendTo);
            d->predicateAction[ActionID::kSendTo] = sendToAct;
        }
    }
    return true;
}

void SendToMenuScene::updateState(QMenu *parent)
{
    // TODO(xust)
}

bool SendToMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

    if (d->predicateAction.key(action).isEmpty())
        return false;

    d->handleActionTriggered(action);
    return true;
}

dfmbase::AbstractMenuScene *SendToMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SendToMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

dfmbase::AbstractMenuScene *SendToMenuCreator::create()
{
    return new SendToMenuScene();
}

SendToMenuScenePrivate::SendToMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kSendTo] = tr("Send to");
    predicateName[ActionID::kSendToBluetooth] = tr("Bluetooth");
}

void SendToMenuScenePrivate::addSubActions(QMenu *subMenu)
{
    if (!subMenu)
        return;

    if (DSC_NAMESPACE::BluetoothService::service()->bluetoothEnable()) {
        auto *act = subMenu->addAction(predicateName[ActionID::kSendToBluetooth]);
        act->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, ActionID::kSendToBluetooth);
        if (folderSelected)
            act->setEnabled(false);
        predicateAction[ActionID::kSendToBluetooth] = act;
    }

    using namespace GlobalServerDefines;
    QAction *firstOptical { nullptr };
    QVariantMap opts { { ListOpt::kMounted, true }, { ListOpt::kRemovable, true } };
    auto devs = DeviceManagerInstance.invokeBlockDevicesIdList(opts);
    int idx = 0;
    for (const QString &dev : devs) {
        auto data = DeviceManagerInstance.invokeQueryBlockDeviceInfo(dev);
        QString label = data.value(DeviceProperty::kIdLabel).toString();
        QString mpt = data.value(DeviceProperty::kMountPoint).toString();
        QString devDesc = data.value(DeviceProperty::kDevice).toString();

        if (label.isEmpty())
            ;   // TODO(xust) label might be empty, use size instead
        QAction *act { nullptr };
        if (data.value(DeviceProperty::kOptical).toBool()) {
            act = subMenu->addAction(label);
            if (!firstOptical)
                firstOptical = act;

            auto u = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                         + "/" + qApp->organizationName() + "/discburn/" + devDesc.replace("/", "_"));
            act->setData(u);
        } else {
            if (firstOptical) {
                act = new QAction(label, subMenu);
                subMenu->insertAction(firstOptical, act);
            } else {
                act = subMenu->addAction(label);
            }
            act->setData(QUrl::fromLocalFile(mpt));
        }

        if (act) {
            QString actId = ActionID::kSendToRemovablePrefix + QString::number(idx++);
            predicateAction.insert(actId, act);
            act->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, actId);
        }
    }
}

void SendToMenuScenePrivate::handleActionTriggered(QAction *act)
{
    if (!act)
        return;

    QStringList filePaths;
    for (const auto &url : selectFiles) {
        auto f = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true);
        filePaths << f->absoluteFilePath();
    }
    DSC_USE_NAMESPACE
    QString actId = act->property(ActionPropertyKey::kActionID).toString();
    if (actId == ActionID::kSendToBluetooth) {
        BluetoothService::service()->sendFiles(filePaths);
    } else if (actId.startsWith(ActionID::kSendToRemovablePrefix)) {
        qDebug() << "send files to: " << act->data().toUrl() << ", " << selectFiles;
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy, QApplication::activeWindow()->winId(), selectFiles, act->data().toUrl(), AbstractJobHandler::JobFlag::kNoHint);
    }
}
