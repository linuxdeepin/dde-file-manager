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

#include "mergeddesktopcontroller.h"

#include "dfmevent.h"
#include "dfilewatcher.h"
#include "dfileservices.h"

#include "interfaces/dfileservices.h"
#include "interfaces/dfmstandardpaths.h"
#include "models/mergeddesktopfileinfo.h"

#include <QList>
#include <QStandardPaths>

MergedDesktopController::MergedDesktopController(QObject *parent)
    : DAbstractFileController(parent),
      m_desktopFileWatcher(new DFileWatcher(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(), this))
{
    connect(m_desktopFileWatcher, &DFileWatcher::fileDeleted, this, &MergedDesktopController::desktopFilesRemoved);
    connect(m_desktopFileWatcher, &DFileWatcher::subfileCreated, this, &MergedDesktopController::desktopFilesCreated);
    connect(m_desktopFileWatcher, &DFileWatcher::fileMoved, this, &MergedDesktopController::desktopFilesRenamed);
    m_desktopFileWatcher->startWatcher();
}

const DAbstractFileInfoPointer MergedDesktopController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new MergedDesktopFileInfo(event->url()));
}

const QList<DAbstractFileInfoPointer> MergedDesktopController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    if (!dataInitialized) {
        initData();
        dataInitialized = true;
    }

    DUrl currentUrl { event->url() };
    QString path { currentUrl.path() };
    QList<DAbstractFileInfoPointer> infoList;

    auto appendVirtualEntries = [this, &infoList](bool displayEmptyEntry = false, const QStringList & expandedEntries = {}) {
        for (unsigned int i = DMD_PICTURE; i <= DMD_OTHER; i++) {
            DMD_TYPES oneType = static_cast<DMD_TYPES>(i);
            if (!displayEmptyEntry && arrangedFileUrls[oneType].isEmpty()) {
                continue;
            }
            QString entryName = entryNameByEnum(oneType);
            DUrl url(DFMMD_ROOT "entry/" + entryNameByEnum(oneType));
            DAbstractFileInfoPointer infoPtr {
                DFileService::instance()->createFileInfo(this, url)
            };
            infoList.push_back(infoPtr);
            if (expandedEntries.contains(entryName)) {
                appendEntryFiles(infoList, oneType);
            }
        }
    };

    auto appendFolders = [this, &infoList]() {
        for (const DUrl & url : arrangedFileUrls[DMD_FOLDER]) {
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
            infoList.append(info);
        }
    };

    auto makeAndInsertInfo = [this, &infoList](QString urlStr) {
        DUrl entryUrl(urlStr);
        DAbstractFileInfoPointer adeEntryInfoPtr {
            DFileService::instance()->createFileInfo(this, entryUrl)
        };
        infoList.push_back(adeEntryInfoPtr);
    };

    if(currentUrl.scheme() == DFMMD_SCHEME) {
        if (path == QStringLiteral("/")) {
            makeAndInsertInfo(DFMMD_ROOT "entry/");
            makeAndInsertInfo(DFMMD_ROOT "folder/");
            makeAndInsertInfo(DFMMD_ROOT "mergeddesktop/");
        } else if (path.startsWith(QStringLiteral("/entry/"))) {
            if (path == QStringLiteral("/entry/")) {
                appendVirtualEntries();
            } else {
                QString entryName = path.split('/', QString::SkipEmptyParts).last();
                DMD_TYPES entryType = entryTypeByName(entryName);
                appendEntryFiles(infoList, entryType);
            }
        } else if (path == QStringLiteral("/folder/")) {
            appendFolders();
        } else if (path == QStringLiteral("/mergeddesktop/")) {
            QString expandedFolder = currentUrl.fragment();
            QStringList expandedFolders;
            if (!expandedFolder.isEmpty()) {
                expandedFolders = expandedFolder.split(',', QString::SkipEmptyParts);
            }
            appendVirtualEntries(false, expandedFolders);
            appendFolders();
        }
    }

    return infoList;
}

DAbstractFileWatcher *MergedDesktopController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &) const
{
    return new DFileWatcher(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(), nullptr);
}

DUrlList MergedDesktopController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList urlList = event->urlList();
    for (const DUrl &url : urlList) {
        if (url.scheme() == DFMMD_SCHEME) {
            urlList.removeOne(url);
        }
    }
    return DFileService::instance()->moveToTrash(event->sender(), urlList);
}

DUrlList MergedDesktopController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    return DAbstractFileController::pasteFile(
                dMakeEventPointer<DFMPasteEvent>(event->sender(), event->action(),
                                                 DUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()),
                                                 event->fileUrlList())
                );
}

bool MergedDesktopController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    return DFileService::instance()->openInTerminal(event->sender(), DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DesktopPath)));
}

const QString MergedDesktopController::entryNameByEnum(DMD_TYPES singleType)
{
    switch (singleType) {
    case DMD_PICTURE:
        return tr("Pictures");
    case DMD_MUSIC:
        return tr("Music");
    case DMD_APPLICATION:
        return tr("Applications");
    case DMD_VIDEO:
        return tr("Videos");
    case DMD_DOCUMENT:
        return tr("Documents");
    case DMD_OTHER:
        return tr("Others");
    case DMD_FOLDER:
        return "Folders";
    default:
        return "Bug";
    }
}

DMD_TYPES MergedDesktopController::entryTypeByName(QString entryName)
{
    if (entryName == tr("Pictures")) {
        return DMD_PICTURE;
    } else if (entryName == tr("Music")) {
        return DMD_MUSIC;
    } else if (entryName == tr("Applications")) {
        return DMD_APPLICATION;
    } else if (entryName == tr("Videos")) {
        return DMD_VIDEO;
    } else if (entryName == tr("Documents")) {
        return DMD_DOCUMENT;
    } else if (entryName == tr("Others")) {
        return DMD_OTHER;
    }

    qWarning() << "MergedDesktopController::entryTypeByName() cannot match a reasonable result, that can be a bug." << qPrintable(entryName);

    return DMD_OTHER;
}


void MergedDesktopController::desktopFilesCreated(const DUrl &url)
{
    DMD_TYPES typeInfo = checkUrlArrangedType(url);
    arrangedFileUrls[typeInfo].append(url);
    DUrl parentUrl(DFMMD_SCHEME "entry/" + entryNameByEnum(typeInfo) + "/");
    DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::subfileCreated, url);
    DUrl parentUrl2(DFMMD_SCHEME "mergeddesktop/");
    DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::subfileCreated, url);
}

void MergedDesktopController::desktopFilesRemoved(const DUrl &url)
{
    for (unsigned int i = DMD_PICTURE; i <= DMD_OTHER; i++) {
        DMD_TYPES typeInfo = static_cast<DMD_TYPES>(i);
        if (arrangedFileUrls[typeInfo].removeOne(url)) {
            DUrl parentUrl(DFMMD_SCHEME "entry/" + entryNameByEnum(typeInfo) + "/");
            DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::fileDeleted, url);
            DUrl parentUrl2(DFMMD_SCHEME "mergeddesktop/");
            DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::fileDeleted, url);
            return;
        }
    }
}

void MergedDesktopController::desktopFilesRenamed(const DUrl &oriUrl, const DUrl &dstUrl)
{
    for (unsigned int i = DMD_PICTURE; i <= DMD_OTHER; i++) {
        DMD_TYPES typeInfo = static_cast<DMD_TYPES>(i);
        if (arrangedFileUrls[typeInfo].removeOne(oriUrl)) {
            break;
        }
    }
    DMD_TYPES typeInfo = checkUrlArrangedType(dstUrl);
    arrangedFileUrls[typeInfo].append(dstUrl);

    DUrl parentUrl(DFMMD_SCHEME "entry/" + entryNameByEnum(typeInfo) + "/");
    DAbstractFileWatcher::ghostSignal(parentUrl, &DAbstractFileWatcher::fileMoved, oriUrl, dstUrl);
    DUrl parentUrl2(DFMMD_SCHEME "mergeddesktop/");
    DAbstractFileWatcher::ghostSignal(parentUrl2, &DAbstractFileWatcher::fileMoved, oriUrl, dstUrl);
}

void MergedDesktopController::initData() const
{
    QDir desktopDir(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    const QStringList &fileList = desktopDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QString &oneFile : fileList) {
        DUrl oneUrl = DUrl::fromLocalFile(desktopDir.filePath(oneFile));
        DMD_TYPES typeInfo = checkUrlArrangedType(oneUrl);
        arrangedFileUrls[typeInfo].append(oneUrl);
    }

    return;
}

DMD_TYPES MergedDesktopController::checkUrlArrangedType(const DUrl url) const
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (info) {
        QString mimetypeDisplayName = info->mimeTypeDisplayName();
        if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Application"))) {
            return DMD_APPLICATION;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Image"))) {
            return DMD_PICTURE;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Audio"))) {
            return DMD_MUSIC;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Video"))) {
            return DMD_VIDEO;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Text"))) {
            return DMD_DOCUMENT;
        } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Directory"))) {
            return DMD_FOLDER;
        } else {
            return DMD_OTHER;
        }
    }

    return DMD_OTHER;
}

void MergedDesktopController::appendEntryFiles(QList<DAbstractFileInfoPointer> &infoList, const DMD_TYPES &entryType) const
{
    for (const DUrl & url : arrangedFileUrls[entryType]) {
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
        infoList.append(info);
    }
}
