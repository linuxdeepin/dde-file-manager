// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
