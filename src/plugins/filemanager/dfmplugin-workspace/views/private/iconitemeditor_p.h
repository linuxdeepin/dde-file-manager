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

#include "dfmplugin_workspace_global.h"

#include <DArrowRectangle>

#include <QStack>

class QTextEdit;

DWIDGET_USE_NAMESPACE
namespace dfmplugin_workspace {

class IconItemEditor;
class IconItemEditorPrivate
{
public:
    explicit IconItemEditorPrivate(IconItemEditor *qq);
    ~IconItemEditorPrivate();

    void init();
    bool canDeferredDelete { true };
    QLabel *icon { nullptr };
    QTextEdit *edit { nullptr };
    int editTextStackCurrentIndex { -1 };
    bool disableEditTextStack { false };

    QStack<QString> editTextStack;
    QGraphicsOpacityEffect *opacityEffect { nullptr };
    int maxCharSize { INT_MAX };

    DArrowRectangle *tooltip { nullptr };
    QString validText;
    IconItemEditor *q_ptr;
    Q_DECLARE_PUBLIC(IconItemEditor)
};
}
