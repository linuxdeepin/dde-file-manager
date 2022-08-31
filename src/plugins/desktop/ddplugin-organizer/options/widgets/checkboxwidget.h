/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef CHECKBOXWIDGET_H
#define CHECKBOXWIDGET_H

#include "entrywidget.h"

#include <DCheckBox>

namespace ddplugin_organizer {

class CheckBoxWidget : public EntryWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidget(const QString &text, QWidget *parent = nullptr);
    void setChecked(bool checked = true);
    bool checked() const;
signals:
    void chenged(bool checked);
protected:
     DTK_WIDGET_NAMESPACE::DCheckBox *checkBox = nullptr;
};

}

#endif // CHECKBOXWIDGET_H
