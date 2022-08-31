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
#ifndef ORGANIZATIONGROUP_H
#define ORGANIZATIONGROUP_H

#include "widgets/switchwidget.h"
#include "methodgroup/methodcombox.h"
#include "methodgroup/methodgrouphelper.h"

#include <QWidget>
#include <QVBoxLayout>

namespace ddplugin_organizer {
class OrganizationGroup : public QWidget
{
    Q_OBJECT
public:
    explicit OrganizationGroup(QWidget *parent = nullptr);
    ~OrganizationGroup();
    void reset();
signals:

public slots:
protected slots:
    void checkedChanged(bool);
private:
    SwitchWidget *organizationSwitch = nullptr;
    MethodComBox *methodCombox = nullptr;
    MethodGroupHelper *currentClass = nullptr;
    QVBoxLayout *contentLayout = nullptr;
};
}

#endif // ORGANIZATIONGROUP_H
