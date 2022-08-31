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

#ifndef METHODCOMBOX_H
#define METHODCOMBOX_H

#include "organizer_defines.h"
#include "options/widgets/entrywidget.h"

#include <DComboBox>

#include <QLabel>

namespace ddplugin_organizer {

class MethodComBox : public EntryWidget
{
    Q_OBJECT
public:
    explicit MethodComBox(const QString &title, QWidget *parent = nullptr);
    void initCheckBox();
    void setCurrentMethod(int idx);
    int currentMethod();
signals:
    void methodChanged();
protected:
    QLabel *label = nullptr;
    Dtk::Widget::DComboBox *comboBox = nullptr;
};

}

#endif // METHODCOMBOX_H
