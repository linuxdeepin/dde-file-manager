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

#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <DAbstractDialog>

namespace ddplugin_organizer {

class OptionsWindowPrivate;
class OptionsWindow : public DTK_WIDGET_NAMESPACE::DAbstractDialog
{
    Q_OBJECT
    friend class OptionsWindowPrivate;
public:
    explicit OptionsWindow(QWidget *parent = 0);
    ~OptionsWindow() override;
    bool initialize();
    void moveToCenter(const QPoint &cursorPos);
private:
    OptionsWindowPrivate *d;
};

}

#endif // OPTIONSWINDOW_H
