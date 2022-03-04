/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "iconitemeditor_p.h"
#include "views/iconitemeditor.h"
#include "views/cansetdragtextedit.h"
#include "utils/itemdelegatehelper.h"

#include <QLabel>

DPWORKSPACE_USE_NAMESPACE

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

    edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
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
