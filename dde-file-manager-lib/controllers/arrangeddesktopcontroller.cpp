/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "arrangeddesktopcontroller.h"

#include "dfmevent.h"
#include "dfilewatcher.h"
#include "dfileservices.h"

#include "models/arrangeddesktopfileinfo.h"

#include <QList>
#include <QStandardPaths>

ArrangedDesktopController::ArrangedDesktopController(QObject *parent)
    : DAbstractFileController(parent),
      m_desktopFileWatcher(new DFileWatcher(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(), this))
{
    connect(m_desktopFileWatcher, &DFileWatcher::fileDeleted, this, &ArrangedDesktopController::desktopFilesRemoved);
    connect(m_desktopFileWatcher, &DFileWatcher::subfileCreated, this, &ArrangedDesktopController::desktopFilesCreated);
    m_desktopFileWatcher->startWatcher();
}

const DAbstractFileInfoPointer ArrangedDesktopController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new ArrangedDesktopFileInfo(event->url()));
}

const QList<DAbstractFileInfoPointer> ArrangedDesktopController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    if (!dataInitialized) {
        initData();
        dataInitialized = true;
    }

    DUrl currentUrl { event->url() };
    QString path { currentUrl.path() };
    QList<DAbstractFileInfoPointer> infoList;

    if(currentUrl.scheme() == QStringLiteral("dfmad")) {
        if (path == QStringLiteral("/")) {
            //
        } else if (path.startsWith(QStringLiteral("/entry/"))) {
            if (path == QStringLiteral("/entry/")) {
                for (int i = DAD_PICTURE; i <= DAD_OTHER; i++) {
                    DAD_TYPES oneType = static_cast<DAD_TYPES>(i);
                    qDebug() << arrangedFileUrls[oneType];
                    DUrl url("dfmad:///entry/" + entryNameByEnum(oneType));
                    DAbstractFileInfoPointer adeInfoPtr {
                        DFileService::instance()->createFileInfo(this, url)
                    };
                    infoList.push_back(adeInfoPtr);
                }
            } else {
                QString entryName = path.split('/', QString::SkipEmptyParts).last();
                DAD_TYPES entryType = entryTypeByName(entryName);
                for (const DUrl & url : arrangedFileUrls[entryType]) {
                    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
                    infoList.append(info);
                }
            }
        } else if (path == QStringLiteral("/folder/")) {
            for (const DUrl & url : arrangedFileUrls[DAD_FOLDER]) {
                DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
                infoList.append(info);
            }
        }
    }

    return infoList;
}

DUrlList ArrangedDesktopController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList urlList = event->urlList();
    for (const DUrl &url : urlList) {
        if (url.scheme() == "dfmad") {
            urlList.removeOne(url);
        }
    }
    return DFileService::instance()->moveToTrash(event->sender(), urlList);
}

DUrlList ArrangedDesktopController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    return DAbstractFileController::pasteFile(
                dMakeEventPointer<DFMPasteEvent>(event->sender(), event->action(),
                                                 DUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()),
                                                 event->fileUrlList())
                );
}

const QString ArrangedDesktopController::entryNameByEnum(DAD_TYPES singleType)
{
    switch (singleType) {
    case DAD_PICTURE:
        return tr("Pictures");
    case DAD_MUSIC:
        return tr("Music");
    case DAD_APPLICATION:
        return tr("Applications");
    case DAD_VIDEO:
        return tr("Videos");
    case DAD_DOCUMENT:
        return tr("Documents");
    case DAD_OTHER:
        return tr("Others");
    case DAD_FOLDER:
        return "Folders";
    default:
        return "Bug";
    }
}

DAD_TYPES ArrangedDesktopController::entryTypeByName(QString entryName)
{
    if (entryName == tr("Pictures")) {
        return DAD_PICTURE;
    } else if (entryName == tr("Music")) {
        return DAD_MUSIC;
    } else if (entryName == tr("Applications")) {
        return DAD_APPLICATION;
    } else if (entryName == tr("Videos")) {
        return DAD_VIDEO;
    } else if (entryName == tr("Documents")) {
        return DAD_DOCUMENT;
    } else if (entryName == tr("Others")) {
        return DAD_OTHER;
    }

    qWarning() << "ArrangedDesktopController::entryTypeByName() cannot match a reasonable result, that can be a bug." << qPrintable(entryName);

    return DAD_OTHER;
}


void ArrangedDesktopController::desktopFilesCreated(const DUrl &url)
{
    DAD_TYPES typeInfo = checkUrlArrangedType(url);
    arrangedFileUrls[typeInfo].append(url);
    DUrl parentUrl("dfmad:///entry/" + entryNameByEnum(typeInfo) + "/");
    DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::subfileCreated, url);
}

void ArrangedDesktopController::desktopFilesRemoved(const DUrl &url)
{
    for (int i = DAD_PICTURE; i <= DAD_OTHER; i++) {
        DAD_TYPES typeInfo = static_cast<DAD_TYPES>(i);
        if (arrangedFileUrls[typeInfo].removeOne(url)) {
            DUrl parentUrl("dfmad:///entry/" + entryNameByEnum(typeInfo) + "/");
            DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::fileDeleted, url);
            return;
        }
    }
}

void ArrangedDesktopController::initData() const
{
    QDir desktopDir(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    const QStringList &fileList = desktopDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QString &oneFile : fileList) {
        DUrl oneUrl = DUrl::fromLocalFile(desktopDir.filePath(oneFile));
        DAD_TYPES typeInfo = checkUrlArrangedType(oneUrl);
        arrangedFileUrls[typeInfo].append(oneUrl);
    }

    return;
}

DAD_TYPES ArrangedDesktopController::checkUrlArrangedType(const DUrl url) const
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (info) {
        QString mimetypeDisplayName = info->mimeTypeDisplayName();
        if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Application"))) {
            return DAD_APPLICATION;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Image"))) {
            return DAD_PICTURE;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Audio"))) {
            return DAD_MUSIC;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Video"))) {
            return DAD_VIDEO;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Text"))) {
            return DAD_DOCUMENT;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Directory"))) {
            return DAD_FOLDER;
        } else {
            return DAD_OTHER;
        }
    }

    return DAD_OTHER;
}
