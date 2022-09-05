// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVETOTRASHCONFLICTDIALOG_H
#define MOVETOTRASHCONFLICTDIALOG_H

#include <QWidget>
#include "ddialog.h"
#include "interfaces/durl.h"

DWIDGET_USE_NAMESPACE

class MoveToTrashConflictDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MoveToTrashConflictDialog(QWidget *parent = 0,
                                       const DUrlList& urls = DUrlList());

signals:

public slots:
};

#endif // MOVETOTRASHCONFLICTDIALOG_H
