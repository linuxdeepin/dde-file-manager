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
#include "cansetdragtextedit.h"

#include <private/qtextedit_p.h>

DPWORKSPACE_USE_NAMESPACE

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
    QTextEditPrivate *dd = reinterpret_cast<QTextEditPrivate *>(qGetPtrHelper(d_ptr));
    dd->control->setDragEnabled(bdrag);
}
