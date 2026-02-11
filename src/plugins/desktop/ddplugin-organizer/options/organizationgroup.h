// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZATIONGROUP_H
#define ORGANIZATIONGROUP_H

#include "widgets/switchwidget.h"
#include "widgets/shortcutwidget.h"
#include "methodgroup/methodcombox.h"
#include "methodgroup/methodgrouphelper.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QSpacerItem>

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
    void enableOrganizeChanged(bool);
    void enableHideAllChanged(bool);

private:
    QLayout *buildTypeLayout();
    void initAll();
    void clearlAll();
    void initShortcutWidget();
    void clearShortcutWidget();

private:
    SwitchWidget *organizationSwitch { nullptr };
    SwitchWidget *hideAllSwitch { nullptr };
    ShortcutWidget *shortcutForHide { nullptr };
    MethodComBox *methodCombox { nullptr };
    MethodGroupHelper *currentClass { nullptr };
    QVBoxLayout *contentLayout { nullptr };
    QSpacerItem *spacer1 { nullptr };
    QSpacerItem *spacer2 { nullptr };
};
}

#endif   // ORGANIZATIONGROUP_H
