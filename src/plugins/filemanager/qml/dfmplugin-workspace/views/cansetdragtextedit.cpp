// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cansetdragtextedit.h"

// #include <private/qtextedit_p.h>

using namespace dfmplugin_workspace;

CanSetDragTextEdit::CanSetDragTextEdit(QWidget *parent)
    : DTextEdit(parent)
{
}

CanSetDragTextEdit::CanSetDragTextEdit(const QString &text, QWidget *parent)
    : DTextEdit(text, parent)
{
}

void CanSetDragTextEdit::setDragEnabled(const bool &bdrag)
{
    // QTextEditPrivate *dd = reinterpret_cast<QTextEditPrivate *>(qGetPtrHelper(d_ptr));
    // dd->control->setDragEnabled(bdrag);
}
