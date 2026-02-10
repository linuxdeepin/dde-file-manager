// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include "entrywidget.h"

#include <DSwitchButton>

#include <QLabel>

namespace ddplugin_organizer {

class SwitchWidget : public EntryWidget
{
    Q_OBJECT
public:
    explicit SwitchWidget(const QString &title, QWidget *parent = nullptr);

    void setChecked(bool checked = true);
    bool checked() const;

    void setTitle(const QString &title);
    inline QString title() const { return label->text(); }

signals:
    void checkedChanged(bool checked) const;

protected:
    QLabel *label = nullptr;
    Dtk::Widget::DSwitchButton *switchBtn = nullptr;
};

}

#endif // SWITCHWIDGET_H
