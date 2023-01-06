// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLLER_H
#define MOUNTCONTROLLER_H

#include "dabstractfilecontroller.h"

class MountController : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit MountController(QObject *parent = nullptr);
    ~MountController();

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
};

#endif // MOUNTCONTROLLER_H
