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
#include "abstractfileactions.h"

#include "utils/systempathutil.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

#include <QDebug>

DFMBASE_BEGIN_NAMESPACE

AbstractFileActions::~AbstractFileActions()
{
}

AbstractFileActions::AbstractFileActions(AbstractFileInfoPointer fInfo)
    : AbstractFileInfo(fInfo->url(), new AbstractFileInfoPrivate(this)),
      fileInfo(fInfo)
{
}

QVector<ActionType> AbstractFileActions::menuActionList(AbstractMenu::MenuType type) const
{
    if (!fileInfo) {
        qInfo() << "The current decoration file is invalid ! ";
        return {};
    }

    QVector<ActionType> actionKeys;

    if (type == AbtMenuType::kSpaceArea) {
        actionKeys << ActionType::kActNewFolder
                   << ActionType::kActNewDocument
                   << ActionType::kActSeparator
                   << ActionType::kActSortBy
                   << ActionType::kActOpenAsAdmin
                   << ActionType::kActOpenInTerminal
                   // todo (lee or lym):
                   // << ActionType::kActRefreshView
                   << ActionType::kActSeparator
                   << ActionType::kActPaste
                   << ActionType::kActSelectAll;

    } else if (type == AbtMenuType::kSingleFile) {
        actionKeys << ActionType::kActOpen;

        // current focus dir is system dir
        if (fileInfo->isDir() && SystemPathUtil::instance()->isSystemPath(fileInfo->filePath())) {
            actionKeys << ActionType::kActOpenInNewWindow
                       << ActionType::kActOpenInNewTab
                       << ActionType::kActOpenAsAdmin
                       << ActionType::kActSeparator
                       << ActionType::kActCopy
                       << ActionType::kActSeparator
                       << ActionType::kActCompress
                       << ActionType::kActSeparator;

            // todo (lee or lym): share or unshare

            actionKeys << ActionType::kActCreateSymlink
                       << ActionType::kActSendToDesktop
                       << ActionType::kActSeparator
                       << ActionType::kActOpenInTerminal
                       << ActionType::kActSeparator;

        } else {
            // todo(lee or lym): MenuAction::OpenWith
            if (fileInfo->isDir()) {
                actionKeys << ActionType::kActOpenInNewWindow
                           << ActionType::kActOpenInNewTab
                           << ActionType::kActOpenAsAdmin;
            }

            // todo(lee or lym): MenuAction::MountImage

            actionKeys << ActionType::kActSeparator
                       << ActionType::kActCut
                       << ActionType::kActCopy
                       << ActionType::kActRename;

            // todo(lee or lym): MenuAction::CompleteDeletion
            actionKeys << ActionType::kActDelete;
            actionKeys << ActionType::kActSeparator;

            // todo(lee or lym): 1. dir --> Share or UnShare
            // todo(lee or lym): 2. isArchive file --> Compress action

            actionKeys << ActionType::kActCreateSymlink
                       << ActionType::kActSendToDesktop;

            /* todo(lee or lym): ActionType::kActSendToRemovableDisk,
             * ActionType::kActStageFileForBurning,
             * MenuAction::BookmarkRemove
             * MenuAction::BookmarkRemove
             * MenuAction::AddToBookMark
             */

            if (fileInfo->isDir()) {
                actionKeys << ActionType::kActSeparator
                           << ActionType::kActOpenInTerminal
                           << ActionType::kActSeparator;
            }

            // todo(lee or lym): image --> SetAsWallpaper
            // todo(lee or lym): tag protocol --> TagInfo, TagFilesUseColor
        }

    } else if (type == AbtMenuType::kMultiFiles) {
        actionKeys << ActionType::kActOpen
                   << ActionType::kActOpenWith
                   << ActionType::kActSeparator
                   << ActionType::kActCut
                   << ActionType::kActCopy
                   << ActionType::kActRename
                   << ActionType::kActCompress
                   << ActionType::kActSendToDesktop;

        // todo(lee or lym): SendToRemovableDisk, StageFileForBurning, CompleteDeletion

        actionKeys << ActionType::kActDelete
                   << ActionType::kActSeparator
                   << ActionType::kActProperty;

        // todo(lee or lym): tag protocol --> TagInfo, TagFilesUseColor
    } else if (type == AbtMenuType::kMultiFilesSystemPathIncluded) {
        actionKeys << ActionType::kActOpen
                   << ActionType::kActSeparator
                   << ActionType::kActCopy
                   << ActionType::kActCompress
                   << ActionType::kActSendToDesktop
                   << ActionType::kActSeparator;

        // todo(lee or lym): tag protocol --> TagInfo, TagFilesUseColor
    }
    auto bLst = fileInfo->menuActionList(type);
    actionKeys = bLst + actionKeys;
    return actionKeys;
}

QSet<ActionType> AbstractFileActions::disableMenuActionList() const
{
    QSet<ActionType> list;

    // TODO(lee or lym):
    //    if (!fileInfo->isWritable()) {
    //        list << ActionType::kActNewFolder
    //             << ActionType::kActNewDocument
    //             << ActionType::kActPaste;
    //    }

    //    if (!fileInfo->canRename()) {
    //        list << ActionType::kActCut << ActionType::kActRename << ActionType::kActDelete << ActionType::kActCompleteDeletion;
    //    }

    //    if (fileInfo->isVirtualEntry() || (!fileInfo->isReadable() && !fileInfo->isSymLink())) {
    //        list << ActionType::kActCopy;
    //    }
    return fileInfo->disableMenuActionList();
}

QSharedPointer<AbstractFileInfo> AbstractFileActions::getFileInfo() const
{
    return fileInfo;
}

void AbstractFileActions::setFileInfo(QSharedPointer<AbstractFileInfo> fInfo)
{
    fileInfo = fInfo;
}

DFMBASE_END_NAMESPACE
