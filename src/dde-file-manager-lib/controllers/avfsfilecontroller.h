// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILECONTROLLER_H
#define AVFSFILECONTROLLER_H

#include <QObject>

#include "dabstractfilecontroller.h"

class AVFSFileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit AVFSFileController(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;

    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;

    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const override;

    static DUrl realUrl(const DUrl &url);
    static QString findArchFileRootPath(const DUrl &url);
private:
};

#endif // AVFSFILECONTROLLER_H
