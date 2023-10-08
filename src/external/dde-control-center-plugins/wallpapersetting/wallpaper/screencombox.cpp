// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screencombox.h"

#include <QScreen>
#include <QApplication>

using namespace dcc::widgets;
using namespace dfm_wallpapersetting;

ScreenComBox::ScreenComBox(QFrame *parent) : ComboxWidget(parent)
{
    setTitle(tr("Screen"));
    addBackground();
    connect(this, &ScreenComBox::onIndexChanged, this, &ScreenComBox::selectChanged);

    displayInter = new DisplayInter("com.deepin.daemon.Display",
                                    "/com/deepin/daemon/Display",
                                    QDBusConnection::sessionBus(), this);
    connect(displayInter, &DisplayInter::DisplayModeChanged, this, &ScreenComBox::delay);
    connect(displayInter, &DisplayInter::PrimaryChanged, this, &ScreenComBox::delay);
    connect(qApp, &QApplication::screenAdded, this, &ScreenComBox::delay);
    connect(qApp, &QApplication::screenRemoved, this, &ScreenComBox::delay);

    delayTimer.setInterval(200);
    delayTimer.setSingleShot(true);
    connect(&delayTimer, &QTimer::timeout, this, &ScreenComBox::process);
}

void ScreenComBox::reset()
{
    screens.clear();

    QStringList scNames;
    QString pri = qApp->primaryScreen()->name();
    QString display = addSuffix(pri);

    if (isSingle()) {
        screens.append(pri);
        scNames.append(display);
    } else {
        for (auto sc : qApp->screens()) {
            screens.append(sc->name());
            scNames.append(sc->name() == pri ? display : sc->name());
        }
    }

    setComboxOption(scNames);
    setCurrentText(display);

    setVisible(screens.size() > 1);
}

bool ScreenComBox::isSingle()
{
    int mode = displayInter->GetRealDisplayMode();
    if (mode == 1 || mode == 3)
        return true;

    return false;
}

void ScreenComBox::setCurrentScreen(const QString &name)
{
    int idx = screens.indexOf(name);
    if (idx < 0)
        return;

    setCurrentIndex(idx);
}

void ScreenComBox::selectChanged(int idx)
{
    if (idx < 0 || idx >= screens.size())
        return;

    emit screenChanged(screens.at(idx));
}

void ScreenComBox::delay()
{
    delayTimer.start();
}

void ScreenComBox::process()
{
    reset();
    emit screenReseted();
}

QString ScreenComBox::addSuffix(const QString &name)
{
    return name + QString(tr("(Primary)"));
}
