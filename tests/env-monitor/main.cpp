// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <QDebug>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setApplicationName("env-monitor-test");
    a.setApplicationVersion("1.0.0");
    a.setProductName(QObject::tr("Env Monitor Test"));
    a.setProductIcon(QIcon::fromTheme("preferences-system"));

    a.loadTranslator();

    MainWindow w;
    w.show();

    Dtk::Widget::moveToCenter(&w);

    return a.exec();
}
