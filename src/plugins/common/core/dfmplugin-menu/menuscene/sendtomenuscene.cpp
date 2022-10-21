/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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
#include "menuutils.h"
#include "action_defines.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

#include <QMenu>
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

    QAction *actionSendTo = nullptr;
    QMenu *menuSendTo = nullptr;

    auto sendToActionInit = [&]() {
        actionSendTo = parent->addAction(d->predicateName.value(ActionID::kSendTo));
        d->predicateAction[ActionID::kSendTo] = actionSendTo;
        actionSendTo->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSendTo));

        menuSendTo = new QMenu(parent);
        actionSendTo->setMenu(menuSendTo);
    };

    // create link
    if (d->selectFiles.count() == 1) {
        if (!actionSendTo)
            sendToActionInit();

        QAction *actionLink = menuSendTo->addAction(d->predicateName[ActionID::kCreateSymlink]);
        actionLink->setProperty(ActionPropertyKey::kActionID, ActionID::kCreateSymlink);
        d->predicateAction[ActionID::kCreateSymlink] = actionLink;
    }

    // send to desktop
    if (!d->onDesktop && !d->isFocusOnDDEDesktopFile) {
        if (!actionSendTo)
            sendToActionInit();

        QAction *actionToDesktop = menuSendTo->addAction(d->predicateName[ActionID::kSendToDesktop]);
        actionToDesktop->setProperty(ActionPropertyKey::kActionID, ActionID::kSendToDesktop);
        d->predicateAction[ActionID::kSendToDesktop] = actionToDesktop;
    }

    return AbstractMenuScene::create(parent);
}

void SendToMenuScene::updateState(QMenu *parent)
{
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
            bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << d->focusFile, &urls);
            if (ok && !urls.isEmpty())
                localUrl = urls.at(0);
            const QString &linkName = FileUtils::nonExistSymlinkFileName(localUrl);
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
            bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", urlsTrans, &urls);
            if (ok && !urls.isEmpty())
                urlsTrans = urls;

            for (const QUrl &url : urlsTrans) {
                QString linkName = FileUtils::nonExistSymlinkFileName(url);
                QUrl linkUrl = QUrl::fromLocalFile(desktopPath + "/" + linkName);
                dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, d->windowId, url, linkUrl, false, true);
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
