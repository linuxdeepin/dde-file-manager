// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMPTYTRASHWIDGET_H
#define EMPTYTRASHWIDGET_H

#include "dfmplugin_trash_global.h"
#include <QFrame>

namespace dfmplugin_trash {

class EmptyTrashWidget : public QFrame
{
    Q_OBJECT

public:
    explicit EmptyTrashWidget(QWidget *parrent = nullptr);
signals:
    void emptyTrash();
};

}

#endif   // EMPTYTRASHWIDGET_H
