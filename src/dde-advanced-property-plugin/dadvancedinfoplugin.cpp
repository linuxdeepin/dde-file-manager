// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dadvancedinfoplugin.h"
#include "dadvancedinfowidget.h"
DAdvancedInfoPlugin::DAdvancedInfoPlugin(QObject *parent)
    : PropertyDialogExpandInfoInterface(parent)
{

}

QWidget *DAdvancedInfoPlugin::expandWidget(const QString &file)
{
    return new DAdvancedInfoWidget(nullptr, file);
}

QString DAdvancedInfoPlugin::expandWidgetTitle(const QString &file)
{
    Q_UNUSED(file);
    return "Advanced info";
}
