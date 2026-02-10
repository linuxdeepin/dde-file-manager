// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cansetdragtextedit.h"

#ifndef DFM_UNIT_TEST_DISABLE_QT_PRIVATE
#include <private/qtextedit_p.h>
#endif

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
#ifndef DFM_UNIT_TEST_DISABLE_QT_PRIVATE
    QTextEditPrivate *dd = reinterpret_cast<QTextEditPrivate *>(qGetPtrHelper(d_ptr));
    dd->control->setDragEnabled(bdrag);
#else
    // Fallback implementation for unit tests
    // Use the public API which may have limited functionality
    setAcceptDrops(bdrag);
    Q_UNUSED(bdrag)
#endif
}
