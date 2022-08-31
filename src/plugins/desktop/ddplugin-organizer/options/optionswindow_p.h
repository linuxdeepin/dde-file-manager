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

#ifndef OPTIONSWINDOW_P_H
#define OPTIONSWINDOW_P_H

#include "optionswindow.h"
#include "widgets/switchwidget.h"
#include "sizeslider.h"
#include "organizationgroup.h"

#include <DTitlebar>

#include <QWidget>
#include <QVBoxLayout>

namespace ddplugin_organizer {

class OptionsWindowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit OptionsWindowPrivate(OptionsWindow *qq);
    ~OptionsWindowPrivate();
    bool isAutoArrange();
    void setAutoArrange(bool on);
    void enableChanged(bool enable);
    void autoArrangeChanged(bool on);
public:
    QVBoxLayout *mainLayout = nullptr;
    DTK_WIDGET_NAMESPACE::DTitlebar *titlebar = nullptr;

    QWidget *contentWidget = nullptr;
    QVBoxLayout *contentLayout = nullptr;
    SwitchWidget *autoArrange = nullptr;
    OrganizationGroup *organization = nullptr;
    SizeSlider *sizeSlider = nullptr;
private:
    OptionsWindow *q;
};

}
#endif // OPTIONSWINDOW_P_H
