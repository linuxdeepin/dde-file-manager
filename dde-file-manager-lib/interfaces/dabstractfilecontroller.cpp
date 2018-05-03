/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dabstractfilecontroller.h"
#include "dfmevent.h"

#include <DDesktopServices>

DWIDGET_USE_NAMESPACE

DAbstractFileController::DAbstractFileController(QObject *parent)
    : QObject(parent)
{

}

bool DAbstractFileController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    event->ignore();

    return false;
}

DUrlList DAbstractFileController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    event->ignore();

    return DUrlList();
}

DUrlList DAbstractFileController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    event->ignore();

    return DUrlList();
}

bool DAbstractFileController::restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::mkdir(const QSharedPointer<DFMMkdirEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DDesktopServices::showFileItem(event->url());
}

const QList<DAbstractFileInfoPointer> DAbstractFileController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    const DDirIteratorPointer &iterator = createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(event->sender(), event->url(), event->nameFilters(),
                                                                                                    event->filters(), event->flags()));

    QList<DAbstractFileInfoPointer> list;

    if (iterator) {
        while (iterator->hasNext()) {
            iterator->next();
            list.append(iterator->fileInfo());
        }
    }

    return list;
}

const DAbstractFileInfoPointer DAbstractFileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    event->ignore();

    return DAbstractFileInfoPointer();
}

const DDirIteratorPointer DAbstractFileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    event->ignore();

    return DDirIteratorPointer();
}

bool DAbstractFileController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    event->ignore();
    return false;
}

bool DAbstractFileController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent>& event) const
{
    event->ignore();
    return false;
}

QList<QString> DAbstractFileController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    event->ignore();
    return {QString()};
}

DAbstractFileWatcher *DAbstractFileController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    event->ignore();

    return 0;
}
