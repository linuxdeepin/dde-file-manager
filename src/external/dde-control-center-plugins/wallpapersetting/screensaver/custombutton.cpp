// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custombutton.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

using namespace dcc::widgets;
using namespace dfm_wallpapersetting;

CustomButton::CustomButton(QWidget *parent) : SettingsItem(parent)
{
    addBackground();

    setFixedHeight(48);
    QHBoxLayout *lay = new QHBoxLayout(this);
    auto title = new QLabel(tr("Custom Screensaver"), this);

    lay->addWidget(title, 3, Qt::AlignVCenter);

    auto btn = new QPushButton(tr("Screensaver Setting"));
    lay->addWidget(btn, 7, Qt::AlignVCenter);
    btn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    lay->setContentsMargins(10, 0, 10, 0);

    title->setMinimumWidth(110);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(lay);

    connect(btn, &QPushButton::clicked, this, &CustomButton::clicked);
}
