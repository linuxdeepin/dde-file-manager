// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/clipboardmenuscene_p.h"
#include "action_defines.h"
#include "menuutils.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QVariant>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *ClipBoardMenuCreator::create()
{
    return new ClipBoardMenuScene();
}

ClipBoardMenuScenePrivate::ClipBoardMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kPaste] = tr("&Paste");
    predicateName[ActionID::kCut] = tr("Cu&t");
    predicateName[ActionID::kCopy] = tr("&Copy");
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
    d->treeSelectedUrls = params.value(MenuParamKey::kTreeSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
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
            auto curDirInfo = InfoFactory::create<FileInfo>(d->currentDir);
            if (!curDirInfo)
                return;

            curDirInfo->refresh();

            // 检查是否有传统的剪贴板action或者是否有图像数据
            const QMimeData *mimeData = QApplication::clipboard()->mimeData();
            bool hasValidClipboardData = (ClipBoard::instance()->clipboardAction() != ClipBoard::kUnknownAction)
                    || (mimeData && mimeData->hasImage());

            bool disabled = !hasValidClipboardData
                    || !curDirInfo->isAttributes(OptInfoType::kIsWritable);
            paste->setDisabled(disabled);
        }
    } else {   // update menu by focus fileinfo
        if (auto copy = d->predicateAction.value(ActionID::kCopy)) {
            if (!d->focusFileInfo->isAttributes(OptInfoType::kIsReadable) && !d->focusFileInfo->isAttributes(OptInfoType::kIsSymLink))
                copy->setDisabled(true);
        }

        if (auto cut = d->predicateAction.value(ActionID::kCut)) {
            const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(d->currentDir);
            if (!fileInfo || !fileInfo->isAttributes(OptInfoType::kIsWritable))
                cut->setDisabled(true);
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool ClipBoardMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    QString id = d->predicateAction.key(action);

    // trans url to local
    QList<QUrl> selectedUrlsTemp {};
    if (!d->treeSelectedUrls.isEmpty() && d->selectFiles.count() != d->treeSelectedUrls.count())
        selectedUrlsTemp = d->treeSelectedUrls;
    else
        selectedUrlsTemp = d->selectFiles;
    QList<QUrl> urls {};

    bool ok = UniversalUtils::urlsTransformToLocal(selectedUrlsTemp, &urls);
    if (ok && !urls.isEmpty())
        selectedUrlsTemp = urls;

    if (id == ActionID::kPaste) {
        ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
        if (ClipBoard::kCopyAction == action) {
            dpfSignalDispatcher->publish(GlobalEventType::kCopy, d->windowId, selectedUrlsTemp, d->currentDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr);
        } else if (ClipBoard::kCutAction == action) {
            dpfSignalDispatcher->publish(GlobalEventType::kCutFile, d->windowId, selectedUrlsTemp, d->currentDir, AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr);
            //! todo bug#63441 如果是剪切操作，则禁止跨用户的粘贴操作, 讨论是否应该由下层统一处理?

            // clear clipboard after cutting files from clipboard
            ClipBoard::instance()->clearClipboard();
        } else if (action == ClipBoard::kRemoteCopiedAction) {   // 远程协助
            fmInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
            ClipBoard::setCurUrlToClipboardForRemote(d->currentDir);
        } else if (ClipBoard::kRemoteAction == action) {
            dpfSignalDispatcher->publish(GlobalEventType::kCopy, d->windowId, selectedUrlsTemp, d->currentDir, AbstractJobHandler::JobFlag::kCopyRemote, nullptr);
        } else {
            fmWarning() << "clipboard action:" << action << "    urls:" << selectedUrlsTemp;
        }

    } else if (id == ActionID::kCut) {
        dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, d->windowId, ClipBoard::ClipboardAction::kCutAction, selectedUrlsTemp);
    } else if (id == ActionID::kCopy) {
        dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, d->windowId, ClipBoard::ClipboardAction::kCopyAction, selectedUrlsTemp);
    }

    return AbstractMenuScene::triggered(action);
}
