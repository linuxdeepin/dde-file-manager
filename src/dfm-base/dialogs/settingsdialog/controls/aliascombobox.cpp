// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aliascombobox.h"

#include <QStylePainter>

inline constexpr int kAliasRole = Qt::UserRole + 100;

DWIDGET_USE_NAMESPACE

AliasComboBox::AliasComboBox(QWidget *parent)
    : DComboBox(parent)
{
}

void AliasComboBox::setItemAlias(int index, const QString &alias)
{
    setItemData(index, alias, kAliasRole);
}

QString AliasComboBox::itemAlias(int index) const
{
    return itemData(index, kAliasRole).toString();
}

void AliasComboBox::paintEvent(QPaintEvent *e)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    auto alias = currentData(kAliasRole).toString();
    if (!alias.isEmpty())
        opt.currentText = alias;

    painter.drawComplexControl(QStyle::CC_ComboBox, opt);
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}
