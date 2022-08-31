/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
