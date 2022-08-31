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

#ifndef SIZESLIDER_H
#define SIZESLIDER_H

#include "widgets/contentbackgroundwidget.h"

#include <DSlider>

class QLabel;

namespace ddplugin_organizer {

class SizeSlider : public ContentBackgroundWidget
{
public:
    enum Mode {View, Icon };
public:
    explicit SizeSlider(QWidget *parent = nullptr);
    ~SizeSlider();
    void switchMode(Mode mode);
    inline Mode mode() const { return curMode;}
protected slots:
    void setIconLevel(int);
    void syncIconLevel(int);
    void iconClicked(DTK_WIDGET_NAMESPACE::DSlider::SliderIcons icon, bool checked);
protected:
    void setValue(int);
    void resetToView();
    void resetToIcon();
    int iconLevel();
private:
    DTK_WIDGET_NAMESPACE::DSlider *slider = nullptr;
    QLabel *label = nullptr;
    Mode curMode = View;
};

}

#endif // SIZESLIDER_H
