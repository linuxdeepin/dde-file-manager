// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_workspace_global.h"

#include <DArrowRectangle>

#include <QStack>
#include <QGraphicsOpacityEffect>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

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
    int maxHeight = -1;
    bool useCharCountLimit { false };

    DArrowRectangle *tooltip { nullptr };
    QString validText;
    IconItemEditor *q_ptr;
    Q_DECLARE_PUBLIC(IconItemEditor)
};
}
