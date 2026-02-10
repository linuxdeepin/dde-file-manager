// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYWIDGET_H
#define ENTRYWIDGET_H

#include "contentbackgroundwidget.h"

namespace ddplugin_organizer {

class EntryWidget : public ContentBackgroundWidget
{
    Q_OBJECT
public:
    explicit EntryWidget(QWidget *left, QWidget *right, QWidget *parent = nullptr);
    inline QWidget *widget(bool left) const {
        return left ? leftWidget : rightWidget;
    }

protected:
    QWidget *leftWidget = nullptr;
    QWidget *rightWidget = nullptr;
};
}

#endif // ENTRYWIDGET_H
