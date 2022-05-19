/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "private/clipboardmenuscene_p.h"
#include "action_defines.h"
#include "menuutils.h"

#include "services/common/menu/menu_defines.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *ClipBoardMenuCreator::create()
{
    return new ClipBoardMenuScene();
}

ClipBoardMenuScenePrivate::ClipBoardMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kPaste] = tr("Paste");
    predicateName[ActionID::kCut] = tr("Cut");
    predicateName[ActionID::kCopy] = tr("Copy");
}

ClipBoardMenuScene::ClipBoardMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new ClipBoardMenuScenePrivate(this))
{
}

QString ClipBoardMenuScene::name() const
{
    return ClipBoardMenuCreator::name();
}

bool ClipBoardMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *ClipBoardMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<ClipBoardMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ClipBoardMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kPaste));
        d->predicateAction[ActionID::kPaste] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kPaste));
    } else if (!d->isFocusOnDDEDesktopFile) {
        if (!d->isSystemPathIncluded) {
            QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kCut));
            d->predicateAction[ActionID::kCut] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kCut));
        }

        QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kCopy));
        d->predicateAction[ActionID::kCopy] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kCopy));
    }

    return AbstractMenuScene::create(parent);
}

void ClipBoardMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (d->isEmptyArea) {
        if (auto paste = d->predicateAction.value(ActionID::kPaste)) {
            auto curDirInfo = InfoFactory::create<AbstractFileInfo>(d->currentDir);
            bool disabled = (ClipBoard::instance()->clipboardAction() == ClipBoard::kUnknownAction) || !curDirInfo->isWritable();
            paste->setDisabled(disabled);
        }
    } else if (1 == d->selectFiles.count()) {
        if (auto copy = d->predicateAction.value(ActionID::kCopy)) {
            if (!d->focusFileInfo->isReadable() && !d->focusFileInfo->isSymLink())
                copy->setDisabled(true);
        }

        if (auto cut = d->predicateAction.value(ActionID::kCut)) {
            if (!d->focusFileInfo->canRename())
                cut->setDisabled(true);
        }
    } else {
        // todo(wangcl) disable action?
    }

    AbstractMenuScene::updateState(parent);
}

bool ClipBoardMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    QString id = d->predicateAction.key(action);

    if (id == ActionID::kPaste) {
        ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
        if (ClipBoard::kCopyAction == action) {
            dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy, d->windowId, d->selectFiles, d->currentDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr);
        } else if (ClipBoard::kCutAction == action) {
            dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile, d->windowId, d->selectFiles, d->currentDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr);
            //! todo bug#63441 如果是剪切操作，则禁止跨用户的粘贴操作, 讨论是否应该由下层统一处理?

            // clear clipboard after cutting files from clipboard
            ClipBoard::instance()->clearClipboard();
        } else {
            qWarning() << "clipboard action:" << action << "    urls:" << d->selectFiles;
        }

    } else if (id == ActionID::kCut) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard, d->windowId, ClipBoard::ClipboardAction::kCutAction, d->selectFiles);
    } else if (id == ActionID::kCopy) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard, d->windowId, ClipBoard::ClipboardAction::kCopyAction, d->selectFiles);
    }

    return AbstractMenuScene::triggered(action);
}
