// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sendtomenuscene.h"
#include "private/sendtomenuscene_p.h"
#include "menuutils.h"
#include "action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QApplication>
#include <QFileDialog>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *SendToMenuCreator::create()
{
    return new SendToMenuScene();
}

SendToMenuScene::SendToMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new SendToMenuScenePrivate(this))
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
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const QVariantHash &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();
    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    return AbstractMenuScene::initialize(params);
}

bool SendToMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;
    if (d->isEmptyArea)
        return AbstractMenuScene::create(parent);

    QAction *actionSendTo = parent->addAction(d->predicateName.value(ActionID::kSendTo));
    d->predicateAction[ActionID::kSendTo] = actionSendTo;
    actionSendTo->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSendTo));

    QMenu *menuSendTo = new QMenu(parent);
    actionSendTo->setMenu(menuSendTo);

    bool hasTargetItem = false;

    // create link
    if (d->selectFiles.count() == 1) {
        QAction *actionLink = menuSendTo->addAction(d->predicateName[ActionID::kCreateSymlink]);
        actionLink->setProperty(ActionPropertyKey::kActionID, ActionID::kCreateSymlink);
        d->predicateAction[ActionID::kCreateSymlink] = actionLink;
        hasTargetItem = true;
    }

    // send to desktop
    if (!d->onDesktop && !d->isFocusOnDDEDesktopFile) {
        QAction *actionToDesktop = menuSendTo->addAction(d->predicateName[ActionID::kSendToDesktop]);
        actionToDesktop->setProperty(ActionPropertyKey::kActionID, ActionID::kSendToDesktop);
        d->predicateAction[ActionID::kSendToDesktop] = actionToDesktop;
        hasTargetItem = true;
    }

    if (!d->isFocusOnDDEDesktopFile && !d->isSystemPathIncluded) {
        // udisk
        using namespace GlobalServerDefines;
        auto devs = DevProxyMng->getAllBlockIds(DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable);
        auto dedupedDevs = devs.toSet();
        int idx = 0;
        for (const QString &dev : dedupedDevs) {
            auto &&data = DevProxyMng->queryBlockInfo(dev);
            QString label = DeviceUtils::convertSuitableDisplayName(data);
            QString mpt = data.value(DeviceProperty::kMountPoint).toString();
            QString devDesc = data.value(DeviceProperty::kDevice).toString();

            if (data.value(DeviceProperty::kOptical).toBool()) {
                continue;   // this should be added in burn plugin.
            } else {
                QAction *act = menuSendTo->addAction(label);
                act->setData(QUrl::fromLocalFile(mpt));
                QString actId = ActionID::kSendToRemovablePrefix + QString::number(idx++);
                d->predicateAction.insert(actId, act);
                act->setProperty(ActionPropertyKey::kActionID, actId);
                hasTargetItem = true;
            }
        }
    }
    if (!hasTargetItem)
        actionSendTo->setVisible(false);

    return AbstractMenuScene::create(parent);
}

void SendToMenuScene::updateState(QMenu *parent)
{
    // remove device self action
    if (!d->isEmptyArea) {
        auto actions = parent->actions();
        bool removed = false;
        for (auto act : actions) {
            if (removed)
                break;

            if (act->isSeparator())
                continue;

            auto actId = act->property(ActionPropertyKey::kActionID).toString();

            if (actId == "send-to") {
                auto subMenu = act->menu();
                for (QAction *action : subMenu->actions()) {
                    const QUrl &urlData = action->data().toUrl();
                    if (urlData.isValid() && d->focusFile.toString().startsWith(urlData.toString())) {
                        subMenu->removeAction(action);
                        removed = true;
                        break;
                    }
                }
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool SendToMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actId)) {
        if (actId == ActionID::kCreateSymlink) {
            QUrl localUrl { d->focusFile };
            QList<QUrl> urls {};
            bool ok = UniversalUtils::urlsTransformToLocal({ d->focusFile }, &urls);
            if (ok && !urls.isEmpty())
                localUrl = urls.at(0);
            const QString &linkName = FileUtils::nonExistSymlinkFileName(localUrl, QUrl::fromLocalFile(QDir::currentPath()));
            QString linkPath { QFileDialog::getSaveFileName(nullptr, QObject::tr("Create symlink"), linkName) };
            if (!linkPath.isEmpty()) {
                dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink,
                                             d->windowId,
                                             d->focusFile,
                                             QUrl::fromLocalFile(linkPath),
                                             true,
                                             false);
            }
            return true;
        } else if (actId == ActionID::kSendToDesktop) {
            QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
            QList<QUrl> urlsTrans = d->selectFiles;
            QList<QUrl> urls {};
            bool ok = UniversalUtils::urlsTransformToLocal(urlsTrans, &urls);
            if (ok && !urls.isEmpty())
                urlsTrans = urls;

            for (const QUrl &url : urlsTrans) {
                QString linkName = FileUtils::nonExistSymlinkFileName(url, QUrl::fromLocalFile(desktopPath));
                QUrl linkUrl = QUrl::fromLocalFile(desktopPath + "/" + linkName);
                dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, d->windowId, url, linkUrl, false, true);
            }
            return true;
        } else {
            if (actId.startsWith(ActionID::kSendToRemovablePrefix)) {
                qDebug() << "send files to: " << action->data().toUrl() << ", " << d->selectFiles;
                dpfSignalDispatcher->publish(GlobalEventType::kCopy, QApplication::activeWindow()->winId(), d->selectFiles, action->data().toUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
                return true;
            }
        }
        qWarning() << "action not found, id: " << actId;
        return false;
    } else {
        return AbstractMenuScene::triggered(action);
    }
}

AbstractMenuScene *SendToMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SendToMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

SendToMenuScenePrivate::SendToMenuScenePrivate(SendToMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
    predicateName[ActionID::kSendTo] = tr("Send to");
    predicateName[ActionID::kCreateSymlink] = tr("Create link");
    predicateName[ActionID::kSendToDesktop] = tr("Send to desktop");
}
