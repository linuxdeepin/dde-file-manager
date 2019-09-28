/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmadditionalmenu.h"
#include "dfileservices.h"

#include <QDir>
#include <QMenu>
#include <QMimeDatabase>
#include <XdgDesktopFile>
DFM_BEGIN_NAMESPACE

DFMAdditionalMenu::DFMAdditionalMenu(QObject *parent) : QObject(parent)
{
    QDir oemPath("/usr/share/deepin/dde-file-manager/oem-menuextensions/");
    if (oemPath.exists()) {
        for (const QFileInfo &fileInfo : oemPath.entryInfoList({"*.desktop"})) {
            XdgDesktopFile file;
            file.load(fileInfo.filePath());
            if (!file.isValid()) {
                continue;
            }

            QStringList menuTypes;
            if (file.contains(MENU_TYPE_KEY)) {
                menuTypes = file.value(MENU_TYPE_KEY).toString().split(';', QString::SkipEmptyParts);
                for (const QString &oneType : menuTypes) {
                    if (!AllMenuTypes.contains(oneType)) {
                        menuTypes.removeAll(oneType);
                    }
                }
            } else {
                menuTypes = AllMenuTypes;
            }

            if (menuTypes.isEmpty()) {
                qDebug() << "[OEM Menu Support] Entry will probably not be shown due to empty or have no valid"
                         << MENU_TYPE_KEY << "key in the desktop file.";
                qDebug() << "[OEM Menu Support] Details:" << fileInfo.filePath() << "with entry name" << file.name();
            }

            // the XdgDesktopFile::icon() empty fallback is not really an empty fallback, so we need to check it manually.
            QString iconStr = file.localizedValue("Icon").toString();
            QAction *action = new QAction(iconStr.isEmpty() ? QIcon() : file.icon(), file.name());
            QStringList entryActionList = file.actions();
            if (!entryActionList.isEmpty()) {
                QMenu *menu = new QMenu();
                for (const QString &actionName : entryActionList) {
                    QAction *subAction = new QAction(file.actionIcon(actionName), file.actionName(actionName));
                    connect(subAction, &QAction::triggered, this, [subAction, actionName, file](){
                        QStringList files = subAction->data().toStringList();
                        file.actionActivate(actionName, files);
                    });
                    menu->addAction(subAction);
                }
                action->setMenu(menu);
            }

            connect(action, &QAction::triggered, this, [action, file](){
                QStringList files = action->data().toStringList();
                file.startDetached(files);
            });

            actionList.append(action);
            if (file.contains("MimeType"))
                action->setProperty(MIME_TYPE_KEY.data(), file.mimeTypes());

            for (const QString &oneType : menuTypes) {
                actionListByType[oneType].append(action);
            }
        }
    }
}

void DFMAdditionalMenu::appendParentMineType(const QStringList &parentmimeTypes,  QStringList& mimeTypes)
{
    if (parentmimeTypes.size()==0)
        return;

    for (const QString &mtName : parentmimeTypes) {
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForName(mtName);
        mimeTypes.append(mt.name());
        mimeTypes.append(mt.aliases());
        QStringList pmts = mt.parentMimeTypes();
        appendParentMineType(pmts, mimeTypes);
    }
}

QList<QAction *> DFMAdditionalMenu::actions(const QStringList &files, const QString &currentDir)
{
    Q_UNUSED(currentDir);

    QString menuType = "EmptyArea";
    if (files.size() == 0) {
        return actionListByType[menuType];
    }

    QUrl url;
    if (files.count() == 1) {
        url.setUrl(files.first());
        menuType = QFileInfo(url.toLocalFile()).isDir() ? "SingleDir" : "SingleFile";
    } else {
        menuType = "MultiFileDirs";
    }

    // Add file list data.
    for (QAction * action : actionList) {
        action->setData(files);
        if (action->menu()) {
            for (QAction * subAction : action->menu()->actions()) {
                subAction->setData(files);
            }
        }
    }

    QList<QAction *> actions = actionListByType[menuType];
    if (url.isEmpty())
        return  actions;

    const DAbstractFileInfoPointer &file_info = DFileService::instance()->createFileInfo(this, url);
    if (!file_info)
        return {};

    QStringList fileMimeTypes;
    fileMimeTypes.append(file_info->mimeType().name());
    fileMimeTypes.append(file_info->mimeType().aliases());

    const QMimeType &mt = file_info->mimeType();
    appendParentMineType(mt.parentMimeTypes(), fileMimeTypes);
    fileMimeTypes.removeAll({});

    auto isSupport = [](const QString &mt, const QStringList &fileMimeTypes)->bool{
        foreach(const QString &fmt, fileMimeTypes){
            if (fmt.contains(mt, Qt::CaseInsensitive))
                return true;
        }
        return false;
    };


    for (auto it = actions.begin(); it != actions.end(); ) {
        QAction * action = *it;

        if(action) {
            // MimeType not exist == MimeType=*
            if (action->property(MIME_TYPE_KEY.data()).isNull()) {
                ++it;
                continue;
            }

            QStringList supportMimeTypes =  action->property(MIME_TYPE_KEY.data()).toStringList();
            supportMimeTypes.removeAll({});
            bool match = false; // if MimeType exists but value is empty, action will not show

            for (QString mt : supportMimeTypes) {
                if (fileMimeTypes.contains(mt, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }

                int starPos = mt.indexOf("*");
                if (starPos >=0 && isSupport(mt.left(starPos), fileMimeTypes)) {
                    match = true;
                    break;
                }
            }

            if (!match) {
                it = actions.erase(it);
                continue;
            }
        }

        ++it;
    }

    return actions;
}


DFM_END_NAMESPACE
