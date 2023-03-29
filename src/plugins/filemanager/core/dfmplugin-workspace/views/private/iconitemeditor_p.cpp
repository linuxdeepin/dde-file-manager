// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconitemeditor_p.h"
#include "views/iconitemeditor.h"
#include "views/cansetdragtextedit.h"
#include "utils/itemdelegatehelper.h"

#include <QLabel>

using namespace dfmplugin_workspace;

IconItemEditorPrivate::IconItemEditorPrivate(IconItemEditor *qq)
    : q_ptr(qq)
{
}

IconItemEditorPrivate::~IconItemEditorPrivate()
{
    if (tooltip)
        tooltip->deleteLater();
}

void IconItemEditorPrivate::init()
{
    Q_Q(IconItemEditor);

    icon = new QLabel(q);
    edit = new CanSetDragTextEdit(q);

    icon->setAlignment(Qt::AlignCenter);
    icon->setFrameShape(QFrame::NoFrame);
    icon->installEventFilter(q);

    edit->setWordWrapMode(QTextOption::WrapAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);
    edit->installEventFilter(q);
    edit->setAcceptRichText(false);
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setAcceptDrops(false);
    static_cast<CanSetDragTextEdit *>(edit)->setDragEnabled(false);

    auto vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    q->setLayout(vlayout);

    vlayout->addWidget(icon, 0, Qt::AlignTop | Qt::AlignHCenter);
    vlayout->addSpacing(kIconModeIconSpacing);
    vlayout->addWidget(edit, 0, Qt::AlignTop | Qt::AlignHCenter);

    q->setFrameShape(QFrame::NoFrame);
    q->setFocusProxy(edit);

    QObject::connect(edit, &QTextEdit::customContextMenuRequested, q, &IconItemEditor::popupEditContentMenu);
    QObject::connect(edit, &QTextEdit::textChanged, q, &IconItemEditor::onEditTextChanged, Qt::UniqueConnection);
}
