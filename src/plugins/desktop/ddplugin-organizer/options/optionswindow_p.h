// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    SwitchWidget *enableOrganize = nullptr;
    OrganizationGroup *organization = nullptr;
    SizeSlider *sizeSlider = nullptr;

private:
    OptionsWindow *q;
};

}
#endif   // OPTIONSWINDOW_P_H
