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

#ifndef ARRANGEDDESKTOPCONTROLLER_H
#define ARRANGEDDESKTOPCONTROLLER_H

#include "dabstractfilecontroller.h"

typedef enum {
    DAD_PICTURE, DAD_MUSIC, DAD_APPLICATION, DAD_VIDEO, DAD_DOCUMENT, DAD_OTHER, DAD_FOLDER
} DAD_TYPES;

class DFileWatcher;
class ArrangedDesktopController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit ArrangedDesktopController(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

//    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
//    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
//    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
//    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;
//    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
//    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;

    const static QString entryNameByEnum(DAD_TYPES singleType);
    static DAD_TYPES entryTypeByName(QString entryName);

public slots:
    void desktopFilesChanged(const DUrl &url);

private:
    void initData() const;

    DFileWatcher* m_desktopFileWatcher;
    mutable bool dataInitialized = false;
    mutable QMap<DAD_TYPES, QList<DAbstractFileInfoPointer> > arrangedFileInfos;
};

#endif // ARRANGEDDESKTOPCONTROLLER_H
