// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGCRUMBEDIT_H
#define TAGCRUMBEDIT_H

#include "dfmplugin_tag_global.h"

#include <DCrumbEdit>

namespace dfmplugin_tag {

class TagCrumbEdit : public DTK_WIDGET_NAMESPACE::DCrumbEdit
{
    Q_OBJECT
public:
    explicit TagCrumbEdit(QWidget *parent = nullptr);

    bool isEditing();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void updateHeight();

    bool isEditByDoubleClick { false };
    QTextEdit *edit { nullptr };
};

}

#endif   // TAGCRUMBEDIT_H
