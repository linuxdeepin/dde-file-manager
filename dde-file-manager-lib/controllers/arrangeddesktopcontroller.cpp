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
    connect(m_desktopFileWatcher, &DFileWatcher::fileDeleted, this, &ArrangedDesktopController::desktopFilesChanged);
    connect(m_desktopFileWatcher, &DFileWatcher::subfileCreated, this, &ArrangedDesktopController::desktopFilesChanged);
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
                    qDebug() << arrangedFileInfos[oneType];
                    DUrl url("dfmad:///entry/" + entryNameByEnum(oneType));
                    DAbstractFileInfoPointer adeInfoPtr {
                        DFileService::instance()->createFileInfo(this, url)
                    };
                    infoList.push_back(adeInfoPtr);
                }
            } else {
                QString entryName = path.split('/', QString::SkipEmptyParts).last();
                DAD_TYPES entryType = entryTypeByName(entryName);
                return arrangedFileInfos[entryType];
            }
        } else if (path == QStringLiteral("/folder/")) {
            return arrangedFileInfos[DAD_FOLDER];
        }
    }

    return infoList;
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


void ArrangedDesktopController::desktopFilesChanged(const DUrl &url)
{
    Q_UNUSED(url)
}

void ArrangedDesktopController::initData() const
{
    QDir desktopDir(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    const QStringList &fileList = desktopDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QString &oneFile : fileList) {
        DUrl oneUrl = DUrl::fromLocalFile(desktopDir.filePath(oneFile));
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, oneUrl);
        if (info) {
            QString mimetypeDisplayName = info->mimeTypeDisplayName();
            if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Application"))) {
                arrangedFileInfos[DAD_APPLICATION].append(info);
            } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Image"))) {
                arrangedFileInfos[DAD_PICTURE].append(info);
            } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Audio"))) {
                arrangedFileInfos[DAD_MUSIC].append(info);
            } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Video"))) {
                arrangedFileInfos[DAD_VIDEO].append(info);
            } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Text"))) {
                arrangedFileInfos[DAD_DOCUMENT].append(info);
            } else if (mimetypeDisplayName.startsWith(qApp->translate("MimeTypeDisplayManager", "Directory"))) {
                arrangedFileInfos[DAD_FOLDER].append(info);
            } else {
                arrangedFileInfos[DAD_OTHER].append(info);
            }
        }
    }

    for (int i = DAD_PICTURE; i <= DAD_OTHER; i++) {
        DAD_TYPES oneType = static_cast<DAD_TYPES>(i);
        qDebug() << arrangedFileInfos[oneType];
    }

    return;
}
