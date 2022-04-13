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
#include "private/fileoperatormenuscene_p.h"
#include "action_defines.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/properties.h>
#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

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
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();

    if (!d->isEmptyArea) {
        if (d->selectFiles.isEmpty() || !d->focusFile.isValid() || !d->currentDir.isValid()) {
            qDebug() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
            return false;
        }

        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
    }

    return true;
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

    tempAction = parent->addAction(d->predicateName.value(ActionID::kRename));
    d->predicateAction[ActionID::kRename] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRename));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kDelete));
    d->predicateAction[ActionID::kDelete] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDelete));

    return true;
}

void FileOperatorMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (d->isEmptyArea)
        return;

    // delete
    if (auto delAction = d->predicateAction.value(ActionID::kDelete)) {
        if (!d->focusFileInfo->isWritable() && !d->focusFileInfo->isFile() && !d->focusFileInfo->isSymLink())
            delAction->setDisabled(true);
    }

    if (1 == d->selectFiles.count()) {
        // rename
        if (auto rename = d->predicateAction.value(ActionID::kRename))
            if (!d->indexFlags.testFlag(Qt::ItemIsEditable))
                rename->setDisabled(true);

        // todo(wangcl) disable open?

    } else {
        // todo(wangcl) disable rename?

        // open
        if (auto open = d->predicateAction.value(ActionID::kOpen)) {

            // app support mime types
            QStringList supportedMimeTypes;
            QMimeType fileMimeType = d->focusFileInfo->fileMimeType();
            QString defaultAppDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(fileMimeType.name());
            QSettings desktopFile(defaultAppDesktopFile, QSettings::IniFormat);
            desktopFile.setIniCodec("UTF-8");
            Properties mimeTypeProperties(defaultAppDesktopFile, "Desktop Entry");
            supportedMimeTypes = mimeTypeProperties.value("MimeType").toString().split(';');
            supportedMimeTypes.removeAll("");

            QString errString;
            QList<QUrl> redirectedUrls;

            for (auto url : d->selectFiles) {
                auto info = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true, &errString);
                if (Q_UNLIKELY(info.isNull())) {
                    qDebug() << errString;
                    break;
                }

                // if the suffix is the same, it can be opened with the same application
                if (info->suffix() != d->focusFileInfo->suffix()) {

                    QStringList mimeTypeList { info->mimeTypeName() };
                    QUrl parentUrl = info->parentUrl();
                    auto parentInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true, &errString);
                    if (!info.isNull()) {
                        mimeTypeList << parentInfo->mimeTypeName();
                    }

                    bool matched = false;
                    // or,the application suooprt mime type contains the type of the url file mime type
                    for (const QString &oneMimeType : mimeTypeList) {
                        if (supportedMimeTypes.contains(oneMimeType)) {
                            matched = true;
                            break;
                        }
                    }

                    // disable open action when there are different opening methods
                    if (!matched) {
                        open->setDisabled(true);
                        break;
                    }
                }
            }
        }
    }
}

bool FileOperatorMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();

    // open
    if (actionId == ActionID::kOpen) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles, d->windowId, d->selectFiles);
        return true;
    }

    // rename
    if (actionId == ActionID::kRename) {
    }

    // delete
    if (actionId == ActionID::kDelete) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kMoveToTrash, d->windowId, d->selectFiles, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    }

    // TODO(Lee or others):
    return false;
}
