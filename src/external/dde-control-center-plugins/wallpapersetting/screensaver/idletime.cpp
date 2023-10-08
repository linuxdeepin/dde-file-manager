// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "idletime.h"

#include <QCheckBox>
#include <QVBoxLayout>

using namespace dfm_wallpapersetting;
DWIDGET_USE_NAMESPACE
using namespace dcc::widgets;

IdleTime::IdleTime(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QVBoxLayout *bgGpLayout = new QVBoxLayout;
    bgGpLayout->setContentsMargins(0, 0, 0, 0);
    bkg = new DBackgroundGroup(bgGpLayout, this);
    layout->addWidget(bkg);

    bkg->setItemSpacing(1);
    bkg->setItemMargins(QMargins(0, 0, 0, 0));
    bkg->setBackgroundRole(QPalette::Window);
    bkg->setUseWidgetBackground(false);

    combox = new ComboxWidget(tr("Idle Time"));
    bgGpLayout->addWidget(combox);

    pwdCheck = new QCheckBox(tr("Require a password on wakeup"), bkg);
    pwdCheck->setFixedHeight(combox->height());
    bgGpLayout->addWidget(pwdCheck);

    connect(pwdCheck, &QCheckBox::clicked, this, &IdleTime::setIsLock);
    connect(combox, &ComboxWidget::onIndexChanged, this, &IdleTime::setIdle);
}

IdleTime::~IdleTime()
{

}

void IdleTime::reset()
{
    auto ava = availableTime();
    auto cur = getCurrentIdle();
    if (cur > 0 && !ava.contains(cur))
        ava.insert(0, cur);

    valueList = ava;

    int curIndex = valueList.indexOf(cur);
    if (curIndex < 0 || curIndex >= valueList.size())
        curIndex = 0;

    combox->setComboxOption(translateText(valueList));
    combox->setCurrentIndex(curIndex);

    pwdCheck->setChecked(getIsLock());
}

QVector<int> IdleTime::availableTime()
{
    static QVector<int> policy { 60, 300, 600, 900, 1800, 3600, 0 };
    return policy;
}

QString IdleTime::timeFormat(int second)
{
    if (second == 0)
        return tr("Never");

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

QStringList IdleTime::translateText(const QVector<int> &list)
{
    QStringList ret;
    for (int v : list)
        ret.append(timeFormat(v));
    return ret;
}

void IdleTime::setIdle(int idx)
{
    if (idx < 0 || idx >= valueList.size())
        return;

    emit setCurrentIdle(valueList.at(idx));
}
