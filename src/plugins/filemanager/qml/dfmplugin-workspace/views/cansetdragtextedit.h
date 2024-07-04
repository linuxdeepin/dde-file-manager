// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANSETDRAGTEXTEDIT_H
#define CANSETDRAGTEXTEDIT_H

#include "dfmplugin_workspace_global.h"

#include <DTextEdit>

DWIDGET_USE_NAMESPACE
namespace dfmplugin_workspace {

class CanSetDragTextEdit : public DTextEdit
{
    Q_OBJECT
    friend class ExpandedItem;

public:
    explicit CanSetDragTextEdit(QWidget *parent = nullptr);
    explicit CanSetDragTextEdit(const QString &text, QWidget *parent = nullptr);
    //set QTextEdit can drag
    void setDragEnabled(const bool &bdrag);
};

}

#endif   // CANSETDRAGTEXTEDIT_H
