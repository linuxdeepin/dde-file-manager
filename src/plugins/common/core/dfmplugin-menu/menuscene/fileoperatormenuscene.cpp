// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/fileoperatormenuscene_p.h"
#include "action_defines.h"
#include "menuutils.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QVariant>
#include <QSettings>
#include <QFileDialog>
#include <QGuiApplication>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *FileOperatorMenuCreator::create()
{
    return new FileOperatorMenuScene();
}

FileOperatorMenuScenePrivate::FileOperatorMenuScenePrivate(FileOperatorMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kOpen] = tr("Open");
    predicateName[ActionID::kRename] = tr("Rename");
    predicateName[ActionID::kDelete] = tr("Delete");
    predicateName[ActionID::kEmptyTrash] = tr("Empty Trash");
    predicateName[ActionID::kSetAsWallpaper] = tr("Set as wallpaper");
}

FileOperatorMenuScene::FileOperatorMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new FileOperatorMenuScenePrivate(this))
{
}

QString FileOperatorMenuScene::name() const
{
    return FileOperatorMenuCreator::name();
}

bool FileOperatorMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *FileOperatorMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<FileOperatorMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool FileOperatorMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea)
        return true;

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kOpen));
    d->predicateAction[ActionID::kOpen] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpen));

    if (d->selectFiles.count() == 1) {
        auto focusFileInfo = d->focusFileInfo;
        if (d->focusFileInfo->isAttributes(OptInfoType::kIsSymLink)) {
            const auto &targetFile = d->focusFileInfo->pathOf(PathInfoType::kSymLinkTarget);
            auto targetFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(targetFile));
            if (targetFileInfo && targetFileInfo->exists())
                focusFileInfo = targetFileInfo;
        }

        const auto mimeType = focusFileInfo->nameOf(NameInfoType::kMimeTypeName);
        QList<QVariant> supportedTypes = { "image/jpeg", "image/png", "image/bmp", "image/tiff", "image/gif" };
        if (supportedTypes.contains(mimeType) && focusFileInfo->isAttributes(OptInfoType::kIsReadable)) {
            tempAction = parent->addAction(d->predicateName.value(ActionID::kSetAsWallpaper));
            d->predicateAction[ActionID::kSetAsWallpaper] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSetAsWallpaper));
        }
    }

    if (FileUtils::isTrashDesktopFile(d->focusFile)) {
        tempAction = parent->addAction(d->predicateName.value(ActionID::kEmptyTrash));
        d->predicateAction[ActionID::kEmptyTrash] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kEmptyTrash));
    } else if (!d->isFocusOnDDEDesktopFile && !d->isSystemPathIncluded) {
        tempAction = parent->addAction(d->predicateName.value(ActionID::kRename));
        d->predicateAction[ActionID::kRename] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRename));

        tempAction = parent->addAction(d->predicateName.value(ActionID::kDelete));
        d->predicateAction[ActionID::kDelete] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDelete));
    }

    return AbstractMenuScene::create(parent);
}

void FileOperatorMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (d->isEmptyArea)
        return;

    if (FileUtils::isTrashDesktopFile(d->focusFile)) {
        if (auto clearTrash = d->predicateAction.value(ActionID::kEmptyTrash)) {
            auto info = InfoFactory::create<FileInfo>(FileUtils::trashRootUrl());
            if (info->countChildFile() <= 0)
                clearTrash->setDisabled(true);
        }
    }

    // delete
    if (auto delAction = d->predicateAction.value(ActionID::kDelete)) {
        if (!d->focusFileInfo->canAttributes(CanableInfoType::kCanDelete)
            || (!d->focusFileInfo->isAttributes(OptInfoType::kIsWritable)
                && !d->focusFileInfo->isAttributes(OptInfoType::kIsFile)
                && !d->focusFileInfo->isAttributes(OptInfoType::kIsSymLink)
                && d->focusFileInfo->countChildFile() > 0))
            delAction->setDisabled(true);
    }

    // rename
    if (auto rename = d->predicateAction.value(ActionID::kRename)) {
        if (!d->focusFileInfo->canAttributes(CanableInfoType::kCanRename) || !d->indexFlags.testFlag(Qt::ItemIsEditable))
            rename->setDisabled(true);
    }
    // open menu by focus fileinfo, so do not compare other files
    return AbstractMenuScene::updateState(parent);
}

bool FileOperatorMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return AbstractMenuScene::triggered(action);

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();

    // open
    if (actionId == ActionID::kOpen) {
        if (!d->onDesktop && 1 == d->selectFiles.count() && d->focusFileInfo->isAttributes(OptInfoType::kIsDir)) {
            QUrl cdUrl = d->focusFile;
            FileInfoPointer infoPtr = InfoFactory::create<FileInfo>(cdUrl);
            if (infoPtr && infoPtr->isAttributes(OptInfoType::kIsSymLink))
                cdUrl = QUrl::fromLocalFile(infoPtr->pathOf(PathInfoType::kSymLinkTarget));

            if (Application::instance()->appAttribute(Application::kAllwayOpenOnNewWindow).toBool()) {
                dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, cdUrl);
            } else {
                dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, d->windowId, cdUrl);
            }
        } else {
            dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, d->windowId, d->selectFiles);
        }

        return true;
    }

    // rename
    if (actionId == ActionID::kRename) {
    }

    // delete
    if (actionId == ActionID::kDelete) {
        if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, d->windowId, d->selectFiles, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        } else {
            dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, d->windowId, d->selectFiles, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        }

        return true;
    }

    // clear trash
    if (actionId == ActionID::kEmptyTrash) {
        dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash,
                                     d->windowId,
                                     QList<QUrl>(),
                                     AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
        return true;
    }

    // set as wallpaper
    if (actionId == ActionID::kSetAsWallpaper) {
        if (UrlRoute::isVirtual(d->focusFile)) {
            const auto &localFile = d->focusFileInfo->pathOf(PathInfoType::kAbsoluteFilePath);
            FileUtils::setBackGround(localFile);
        } else {
            FileUtils::setBackGround(d->focusFile.toLocalFile());
        }

        return true;
    }

    // TODO(Lee or others):
    return AbstractMenuScene::triggered(action);
}
