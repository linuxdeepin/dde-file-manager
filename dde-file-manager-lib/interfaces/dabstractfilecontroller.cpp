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

class DefaultDiriterator : public DDirIterator
{
public:
    DefaultDiriterator(const DAbstractFileController* controller, const QSharedPointer<DFMCreateDiriterator> &event) ;

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    mutable QList<DAbstractFileInfoPointer> m_children;
    DAbstractFileController* m_controller;
    mutable QSharedPointer<DFMCreateDiriterator> m_event;
    int m_current = -1;
};

DefaultDiriterator::DefaultDiriterator(const DAbstractFileController *controller, const QSharedPointer<DFMCreateDiriterator> &event)
{
    m_controller = const_cast<DAbstractFileController*>(controller);
    m_event = event;
}

DUrl DefaultDiriterator::next()
{
    return m_children.at(++m_current)->fileUrl();
}

bool DefaultDiriterator::hasNext() const
{
    if (m_event) {
        m_children = m_controller->getChildren(m_event);
        m_event.clear();
    }
    return m_current + 1 < m_children.count();
}

QString DefaultDiriterator::fileName() const
{
    return m_children.at(m_current)->fileName();
}

DUrl DefaultDiriterator::fileUrl() const
{
    return m_children.at(m_current)->fileUrl();
}

const DAbstractFileInfoPointer DefaultDiriterator::fileInfo() const
{
    return m_children.at(m_current);
}

DUrl DefaultDiriterator::url() const
{
    return m_event->fileUrl();
}

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

bool DAbstractFileController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::decompressFileHere(const QSharedPointer<DFMDecompressEvent> &event) const
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

bool DAbstractFileController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
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
    const_cast<DAbstractFileController*>(this)->setProperty("_d_call_createDirIterator_in_DAbstractFileController::getChildren", true);

    const DDirIteratorPointer &iterator = createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(event->sender(), event->url(), event->nameFilters(),
                                                                                                    event->filters(), event->flags()));

    const_cast<DAbstractFileController*>(this)->setProperty("_d_call_createDirIterator_in_DAbstractFileController::getChildren", false);

    QList<DAbstractFileInfoPointer> list;

    if (iterator) {
        while (iterator->hasNext()) {
            iterator->next();
            list.append(iterator->fileInfo());
        }
    }

    return list;
}

const DAbstractFileInfoPointer DAbstractFileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    event->ignore();

    return DAbstractFileInfoPointer();
}

const DDirIteratorPointer DAbstractFileController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    if (property("_d_call_createDirIterator_in_DAbstractFileController::getChildren").toBool()) {
        event->ignore();

        return DDirIteratorPointer();
    }

//    const QList<DAbstractFileInfoPointer> &children = getChildren(event);

    return DDirIteratorPointer(new DefaultDiriterator(this, event));
}

bool DAbstractFileController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    event->ignore();

    return false;
}

bool DAbstractFileController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
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

    return nullptr;
}

DFM_NAMESPACE::DFileDevice *DAbstractFileController::createFileDevice(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    event->ignore();

    return nullptr;
}

DFM_NAMESPACE::DFileHandler *DAbstractFileController::createFileHandler(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    event->ignore();

    return nullptr;
}

DFM_NAMESPACE::DStorageInfo *DAbstractFileController::createStorageInfo(const QSharedPointer<DFMUrlBaseEvent> &event) const
{
    event->ignore();

    return nullptr;
}

bool DAbstractFileController::setExtraProperties(const QSharedPointer<DFMSetFileExtraProperties> &event) const
{
    const auto && ep = event->extraProperties();
    const QStringList &tag_name_list = ep.value("tag_name_list").toStringList();

    return setFileTags(dMakeEventPointer<DFMSetFileTagsEvent>(event->sender(), event->url(), tag_name_list));
}
