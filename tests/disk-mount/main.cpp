// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DMainWindow>
#include <QLayout>
#include <QPushButton>
#include "widgets/devicelist.h"
#include "device/dockitemdatamanager.h"

int main(int argc, char **argv)
{
    Dtk::Widget::DApplication app(argc, argv);

    Dtk::Widget::DMainWindow win;
    win.setFixedSize(500, 500);
    QWidget *mainWid = new QWidget(&win);
    win.setCentralWidget(mainWid);
    QHBoxLayout *mainLay = new QHBoxLayout(mainWid);

    // right operation area
    QPushButton *dockIcon = new QPushButton(&win);
    dockIcon->setIcon(QIcon::fromTheme("drive-removable-dock-symbolic"));
    QPushButton *ejectAll = new QPushButton("eject all", &win);
    QVBoxLayout *operationLay = new QVBoxLayout;
    operationLay->addWidget(ejectAll);
    operationLay->addWidget(dockIcon);
    operationLay->addSpacerItem(new QSpacerItem(1, 500));

    QObject::connect(ejectAll, &QPushButton::clicked,
                     &win, [] { DockItemDataManager::instance()->ejectAll(); });

    dockIcon->setVisible(false);
    QObject::connect(DockItemDataManager::instance(), &DockItemDataManager::requesetSetDockVisible,
                     &win, [=](auto visible) { dockIcon->setVisible(visible); });

    // left device area
    QWidget *devWid = new QWidget(&win);
    devWid->setAutoFillBackground(true);
    devWid->setFixedWidth(320);
    devWid->setObjectName("devWid");
    devWid->setStyleSheet("#devWid {border: 1px solid red;}");
    QHBoxLayout *devLay = new QHBoxLayout(devWid);
    devLay->setContentsMargins(0, 10, 0, 0);
    devLay->setSpacing(0);
    DeviceList *devList = new DeviceList(mainWid);
    devLay->addWidget(devList, 0, Qt::AlignTop);

    mainLay->addWidget(devWid);
    mainLay->addLayout(operationLay);
    win.show();

    return app.exec();
}
