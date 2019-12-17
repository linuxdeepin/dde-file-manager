/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfmiconbutton.h"

#include <QPainter>
#include <QApplication>

DFM_BEGIN_NAMESPACE

DFMIconButton::DFMIconButton(QWidget *parent)
    : QAbstractButton (parent)
{

}

DFMIconButton::DFMIconButton(const QString & iconFromThemeName, QWidget *parent)
    : DFMIconButton (parent)
{
    setIcon(QIcon::fromTheme(iconFromThemeName));
}

DFMIconButton::~DFMIconButton()
{
    //
}

void DFMIconButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    QPixmap pm = icon().pixmap(iconSize());
    QPainter pa(&pm);
    pa.setCompositionMode(QPainter::CompositionMode_SourceIn);

    if (isChecked()) {
        pa.fillRect(pm.rect(), qApp->palette().color(QPalette::Active, QPalette::Highlight));
    } else {
        pa.fillRect(pm.rect(), qApp->palette().color(QPalette::Active, QPalette::Text));
    }

    painter.drawPixmap((rect().width() - pm.rect().width()) / 2,
                           (rect().height() - pm.rect().height()) / 2,
                           pm);
}

DFM_END_NAMESPACE
