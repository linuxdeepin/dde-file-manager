// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTITEMEDITOR_H
#define LISTITEMEDITOR_H

#include "dfmplugin_workspace_global.h"

#include <DArrowRectangle>

#include <QLineEdit>

namespace dfmplugin_workspace {

class ListItemEditor : public QLineEdit
{
    Q_OBJECT
public:
    explicit ListItemEditor(QWidget *parent = nullptr);
    ~ListItemEditor();
    void select(const QString &part);
    inline void setMaxCharSize(int l)
    {
        if (l > 0)
            theMaxCharSize = l;
    }

    inline int maxCharSize() const
    {
        return theMaxCharSize;
    }

    inline void setCharCountLimit()
    {
        useCharCount = true;
    }

signals:
    void inputFocusOut();

public slots:
    void showAlertMessage(const QString &text, int duration = 3000);

protected:
    static DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();

    bool event(QEvent *ee) override;

private slots:
    void onEditorTextChanged(const QString &text);

private:
    void init();

private:
    int theMaxCharSize { INT_MAX };
    bool useCharCount { false };
    DTK_WIDGET_NAMESPACE::DArrowRectangle *tooltip { nullptr };
};
}
#endif   // LISTITEMEDITOR_H
