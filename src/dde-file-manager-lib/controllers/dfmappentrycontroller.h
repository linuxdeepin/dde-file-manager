// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMAPPENTRYCONTROLLER_H
#define DFMAPPENTRYCONTROLLER_H

#include "dabstractfilecontroller.h"

class DFMAppEntryController: public DAbstractFileController
{
    Q_OBJECT
public:
    explicit DFMAppEntryController(QObject *parent = nullptr): DAbstractFileController (parent) {}
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;
    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;

    static DUrl localToAppEntry(const DUrl &local);
};

#endif // DFMAPPENTRYCONTROLLER_H
