/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "vaultactivestartview.h"

#include <DIconButton>
#include <DLabel>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultActiveStartView::VaultActiveStartView(QWidget *parent)
    : QWidget(parent)
{
    //! 标题
    DLabel *pLabel1 = new DLabel(tr("File Vault"), this);
    QFont font = pLabel1->font();
    font.setPixelSize(18);
    pLabel1->setFont(font);
    pLabel1->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel2 = new DLabel(tr("Create your secure private space") + '\n' + tr("Advanced encryption technology") + '\n' + tr("Convenient and easy to use"), this);
    pLabel2->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel3 = new DLabel();
    pLabel3->setPixmap(QIcon::fromTheme("dfm_vault_active_start").pixmap(88, 100));
    pLabel3->setAlignment(Qt::AlignHCenter);

    startBtn = new QPushButton(tr("Create"), this);
    connect(startBtn, &QPushButton::clicked,
            this, &VaultActiveStartView::slotStartBtnClicked);

    //! 布局
    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel1);
    play->addSpacing(5);
    play->addWidget(pLabel2);
    play->addSpacing(15);
    play->addWidget(pLabel3);
    play->addStretch();
    play->addWidget(startBtn);
}

void VaultActiveStartView::slotStartBtnClicked()
{
    emit sigAccepted();
}
