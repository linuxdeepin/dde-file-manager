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
#include "dfmvaultfileview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

VaultHeaderView::VaultHeaderView(QWidget *parent)
    : QWidget (parent)
{
    QLabel * lb = new QLabel("Vault", this);
    QFont font = lb->font();
    font.setBold(true);
    font.setPixelSize(24);
    lb->setFont(font);

    DIconButton * menuBtn = new DIconButton(DStyle::SP_TitleBarMenuButton, this);
    menuBtn->setFlat(true);
    menuBtn->setFixedSize(QSize(32, 32));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(lb);
    layout->addStretch();
    layout->addWidget(menuBtn);
}

// --------------------------------------------

DFMVaultFileView::DFMVaultFileView(QWidget *parent)
    : DFileView(parent)
{
    int index = this->addHeaderWidget(new VaultHeaderView(this));
    Q_UNUSED(index);
}
