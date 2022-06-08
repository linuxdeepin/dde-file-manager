/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "sendtodiscmenuscene.h"
#include "sendtodiscmenuscene_p.h"
#include "utils/burnhelper.h"
#include "events/burneventreceiver.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QMenu>
#include <QProcess>

DPBURN_USE_NAMESPACE
DSC_USE_NAMESPACE
using namespace GlobalServerDefines;

namespace ActionId {
static constexpr char kStageKey[] { "stage-file-to-burning" };
static constexpr char kStagePrex[] { "_stage-file-to-burning-" };
static constexpr char kMountImageKey[] { "mount-image" };
}

SendToDiscMenuScenePrivate::SendToDiscMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void SendToDiscMenuScenePrivate::actionStageFileForBurning(const QString &dev)
{
    if (dev.isEmpty())
        return;
    QUrl dest { BurnHelper::fromBurnFile(dev) };
    QList<QUrl> srcUrls { selectFiles };
    std::transform(srcUrls.begin(), srcUrls.end(), srcUrls.begin(), [](const QUrl &url) -> QUrl {
        if (delegateServIns->isRegisterUrlTransform(url.scheme()))
            return delegateServIns->urlTransform(url);
        return url;
    });

    BurnEventReceiver::instance()->handlePasteTo(srcUrls, dest, true);
}

void SendToDiscMenuScenePrivate::actionMountImage()
{
    qInfo() << "Mount image:" << focusFile;
    QString archiveuri;
    auto info { InfoFactory::create<AbstractFileInfo>(focusFile) };
    if (info && info->canRedirectionFileUrl()) {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(info->redirectedFileUrl().toString()));
        qInfo() << "Mount image redirect the url to:" << info->redirectedFileUrl();
    } else {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(focusFile.toString()));
    }

    QStringList args;
    args << "mount" << archiveuri;
    QProcess *gioproc = new QProcess;
    gioproc->start("gio", args);
    connect(gioproc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=](int ret) {
        if (ret) {
            DialogManagerInstance->showErrorDialog(tr("Mount error: unsupported image format"), QString());
        } else {
            QString doubleEncodedUri { QUrl::toPercentEncoding(focusFile.toEncoded()) };
            doubleEncodedUri = QUrl::toPercentEncoding(doubleEncodedUri);
        }
        // TODO(zhangs): cd to mnt path
        gioproc->deleteLater();
    });
}

void SendToDiscMenuScenePrivate::initDestDevices()
{
    bool filterFlag { false };
    // remove self disc id
    auto &&dataGroup { BurnHelper::discDataGroup() };
    for (const auto &data : dataGroup) {
        QString curDev { BurnHelper::burnDestDevice(currentDir) };
        QString dev { data[DeviceProperty::kDevice].toString() };
        if (curDev != dev)
            destDeviceDataGroup.push_back(data);
        else
            filterFlag = true;
    }

    // only self disc, disable action
    if (filterFlag && destDeviceDataGroup.isEmpty())
        disableStage = true;
}

void SendToDiscMenuScenePrivate::addSubStageActions(QMenu *menu)
{
    Q_ASSERT(menu);

    if (destDeviceDataGroup.size() > 1) {
        for (const auto &data : destDeviceDataGroup) {
            QString label { DeviceUtils::convertSuitableDisplayName(data) };
            QString dev { data[DeviceProperty::kDevice].toString() };
            QAction *act { menu->addAction(label) };
            act->setData(dev);
            QString actId { ActionId::kStagePrex + dev };
            act->setProperty(ActionPropertyKey::kActionID, actId);
            predicateAction.insert(actId, act);
        }
    }
}

AbstractMenuScene *SendToDiscMenuCreator::create()
{
    return new SendToDiscMenuScene();
}

SendToDiscMenuScene::SendToDiscMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SendToDiscMenuScenePrivate(this))
{
}

SendToDiscMenuScene::~SendToDiscMenuScene()
{
}

QString SendToDiscMenuScene::name() const
{
    return SendToDiscMenuCreator::name();
}

bool SendToDiscMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->predicateName.insert(ActionId::kStageKey, QObject::tr("Add to disc"));
    d->predicateName.insert(ActionId::kMountImageKey, QObject::tr("Mount"));

    if (d->selectFiles.isEmpty())
        return false;
    d->initDestDevices();

    QUrl url(d->selectFiles.first());
    QString scheme { url.scheme() };
    if (delegateServIns->isRegisterUrlTransform(scheme))
        url = delegateServIns->urlTransform(url);

    if (url.scheme() != Global::kFile)
        return false;

    return AbstractMenuScene::initialize(params);
}

bool SendToDiscMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    // stage file to disc
    if (!d->destDeviceDataGroup.isEmpty() || d->disableStage) {
        QAction *act { parent->addAction(d->predicateName[ActionId::kStageKey]) };
        act->setProperty(ActionPropertyKey::kActionID, ActionId::kStageKey);
        d->predicateAction.insert(ActionId::kStageKey, act);
        // use menu if has multi otical devs
        if (d->destDeviceDataGroup.size() == 1) {
            QString dev { d->destDeviceDataGroup.first()[DeviceProperty::kDevice].toString() };
            act->setData(dev);
        } else if (d->destDeviceDataGroup.size() > 1) {
            QMenu *stageMenu { new QMenu(parent) };
            d->addSubStageActions(stageMenu);
            if (stageMenu->actions().isEmpty())
                delete stageMenu;
            else
                act->setMenu(stageMenu);
        }
    }

    // mount image
    auto focusInfo { InfoFactory::create<AbstractFileInfo>(d->focusFile) };
    if (focusInfo) {
        static QSet<QString> mountable { "application/x-cd-image", "application/x-iso9660-image" };
        if (mountable.contains(focusInfo->mimeTypeName())) {
            QAction *act { parent->addAction(d->predicateName[ActionId::kMountImageKey]) };
            act->setProperty(ActionPropertyKey::kActionID, ActionId::kMountImageKey);
            d->predicateAction.insert(ActionId::kMountImageKey, act);
        }
    }

    return AbstractMenuScene::create(parent);
}

void SendToDiscMenuScene::updateState(QMenu *parent)
{
    updateStageAction(parent);
    updateMountAction(parent);

    return AbstractMenuScene::updateState(parent);
}

bool SendToDiscMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    QString &&key { action->property(ActionPropertyKey::kActionID).toString() };
    if (key == ActionId::kStageKey || key.startsWith(ActionId::kStagePrex)) {
        QString dev { action->data().toString() };
        d->actionStageFileForBurning(dev);
        return true;
    }

    // Maybe as a event?
    if (key == ActionId::kMountImageKey) {
        d->actionMountImage();
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *SendToDiscMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SendToDiscMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

void SendToDiscMenuScene::updateStageAction(QMenu *parent)
{
    auto actions { parent->actions() };

    QAction *sendToAct { nullptr };
    QAction *stageAct { nullptr };
    for (auto act : actions) {
        QString &&id { act->property(DSC_NAMESPACE::ActionPropertyKey::kActionID).toString() };
        if (id == ActionId::kStageKey)
            stageAct = act;
        if (id == "send-to")
            sendToAct = act;
    }

    if (!stageAct)
        return;

    if (sendToAct && stageAct) {
        actions.removeOne(stageAct);
        parent->insertAction(sendToAct, stageAct);
        parent->removeAction(sendToAct);
        parent->insertAction(stageAct, sendToAct);
    }

    // disable action in self disc
    if (d->disableStage) {
        stageAct->setEnabled(false);
        return;
    }

    stageAct->setEnabled(true);
}

void SendToDiscMenuScene::updateMountAction(QMenu *parent)
{
    auto actions { parent->actions() };

    QAction *openWithAct { nullptr };
    QAction *mountAct { nullptr };
    for (auto act : actions) {
        QString &&id { act->property(DSC_NAMESPACE::ActionPropertyKey::kActionID).toString() };
        if (id == ActionId::kMountImageKey)
            mountAct = act;
        if (id == "open-with")
            openWithAct = act;
    }

    if (openWithAct && mountAct) {
        actions.removeOne(openWithAct);
        parent->insertAction(openWithAct, mountAct);
        parent->removeAction(openWithAct);
        parent->insertAction(mountAct, openWithAct);
    }
}
