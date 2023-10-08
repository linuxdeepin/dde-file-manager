// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intervalcombox.h"

#include <QApplication>
#include <QScreen>

using namespace dcc::widgets;
using namespace dfm_wallpapersetting;

IntervalCombox::IntervalCombox(QFrame *parent) : ComboxWidget(parent)
{
    setTitle(tr("Wallpaper Slideshow"));
    addBackground();

    connect(this, &ComboxWidget::onIndexChanged, this, &IntervalCombox::setInterval);
}

IntervalCombox::~IntervalCombox()
{
}

void IntervalCombox::reset(QString sc)
{
    if (sc.isEmpty()) {
        qCritical() << "screen name is empty.";
        sc = qApp->primaryScreen()->name();
    }

    screen = sc;

    auto ava = availableInterval();
    auto cur = getCurrentSlideShow(screen);
    if (!cur.isEmpty() && !ava.contains(cur))
        ava.insert(1, cur);

    valueList = ava;

    int curIndex = valueList.indexOf(cur);
    if (curIndex < 0 || curIndex >= valueList.size())
        curIndex = 0;

    setComboxOption(translateText(valueList));
    setCurrentIndex(curIndex);
}

QStringList IntervalCombox::availableInterval()
{
    static const QStringList policy {"", "30", "60", "300", "600", "900", "1800", "3600", "login", "wakeup" };
    return policy;
}

QString IntervalCombox::timeFormat(int second)
{
    quint8 s = static_cast<quint8>(second % 60);
    int m = static_cast<int>(second / 60);
    int h = m / 60;
    int d = h / 24;

    m = m % 60;
    h = h % 24;

    QString timeString;

    if (d > 0) {
        timeString.append(QString::number(d)).append(tr("d"));
    }

    if (h > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(h)).append(tr("h"));
    }

    if (m > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(m)).append(tr("m"));
    }

    if (s > 0 || timeString.isEmpty()) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(s)).append(tr("s"));
    }

    return timeString;
}

QStringList IntervalCombox::translateText(const QStringList &list)
{
    QStringList ret;
    for (const QString &time : list) {
        if (time.isEmpty()) {
            ret.append(tr("Never"));
        } else if (time == "login") {
            ret.append(tr("When login"));
        } else if (time == "wakeup") {
            ret.append(tr("When wakeup"));
        } else {
            bool ok = false;
            int t = time.toInt(&ok);
            ret.append(ok ? timeFormat(t) : time);
        }
    }

    return ret;
}

void IntervalCombox::setInterval(int index)
{
    if (index < 0 || index >= valueList.size()) {
        qWarning() << "index is out of range" << index;
        return;
    }

    QString value = valueList.at(index);
    emit setCurrentSlideshow(screen, value);
}
