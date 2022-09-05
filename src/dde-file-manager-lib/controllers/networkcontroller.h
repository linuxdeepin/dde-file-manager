// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H


#include "dabstractfilecontroller.h"

class NetworkController: public DAbstractFileController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController() override;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;
};

#endif // NETWORKCONTROLLER_H
