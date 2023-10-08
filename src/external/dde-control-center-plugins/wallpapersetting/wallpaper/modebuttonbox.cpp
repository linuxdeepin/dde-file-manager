// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modebuttonbox.h"

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

ModeButtonBox::ModeButtonBox(QWidget *parent) : DButtonBox(parent)
{

}

void ModeButtonBox::initialize()
{
    setOrientation(Qt::Horizontal);
    QList<DButtonBoxButton *> list;

    DButtonBoxButton *pic = new DButtonBoxButton(tr("Picture"), this);
    pic->setMinimumWidth(87);
    list.append(pic);

    DButtonBoxButton *clr = new DButtonBoxButton(tr("Solid Color"), this);
    pic->setMinimumWidth(87);
    list.append(clr);

    setButtonList(list, true);

    setId(pic, Picture);
    setId(clr, SolidColor);

    connect(this, &DButtonBox::buttonClicked, this, &ModeButtonBox::onButtonClicked);
}

void ModeButtonBox::switchMode(ModeButtonBox::Mode m)
{
    if (auto btn = button(m)) {
        btn->setChecked(true);
        current = m;
    }
}

void ModeButtonBox::onButtonClicked(QAbstractButton *value)
{
    auto ck = id(value);
    if (ck != current)
        emit switchTo(ck);
}
