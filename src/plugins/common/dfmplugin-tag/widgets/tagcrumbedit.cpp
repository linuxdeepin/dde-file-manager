// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagcrumbedit.h"

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

TagCrumbEdit::TagCrumbEdit(QWidget *parent)
    : DCrumbEdit(parent)
{
    auto doc = QTextEdit::document();
    doc->setDocumentMargin(doc->documentMargin() + 5);
}

bool TagCrumbEdit::isEditing()
{
    return isEditByDoubleClick;
}

void TagCrumbEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    isEditByDoubleClick = true;
    DCrumbEdit::mouseDoubleClickEvent(event);
    isEditByDoubleClick = false;
}
