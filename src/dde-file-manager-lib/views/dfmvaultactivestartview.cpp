/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "dfmvaultactivestartview.h"
#include "accessibility/ac-lib-file-manager.h"

#include <DIconButton>
#include <DLabel>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>


DWIDGET_USE_NAMESPACE

DFMVaultActiveStartView::DFMVaultActiveStartView(QWidget *parent)
    : QWidget(parent)
    , m_pStartBtn(nullptr)
{
    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_ACTIVE_START_WIDGET);

    // 标题
    DLabel *pLabel1 = new DLabel(tr("File Vault"), this);
    AC_SET_ACCESSIBLE_NAME(pLabel1, AC_VAULT_ACTIVE_START_TITLE_LABEL);
    QFont font = pLabel1->font();
    font.setPixelSize(18);
    pLabel1->setFont(font);
    pLabel1->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel2 = new DLabel(tr("Create your secure private space") + '\n' +
                                 tr("Advanced encryption technology") + '\n' +
                                 tr("Convenient and easy to use"), this);
    AC_SET_ACCESSIBLE_NAME(pLabel2, AC_VAULT_ACTIVE_START_CONTENT_LABEL);
    pLabel2->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel3 = new DLabel();
    AC_SET_ACCESSIBLE_NAME(pLabel3, AC_VAULT_ACTIVE_START_PICTURE);
    pLabel3->setPixmap(QIcon::fromTheme("dfm_vault_active_start").pixmap(88, 100));
    pLabel3->setAlignment(Qt::AlignHCenter);

    m_pStartBtn = new QPushButton(tr("Create"), this);
    AC_SET_ACCESSIBLE_NAME(m_pStartBtn, AC_VAULT_ACTIVE_START_BUTTON);
    connect(m_pStartBtn, &QPushButton::clicked,
            this, &DFMVaultActiveStartView::slotStartBtnClicked);

    // 布局
    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel1);
    play->addSpacing(5);
    play->addWidget(pLabel2);
    play->addSpacing(15);
    play->addWidget(pLabel3);
    play->addStretch();
    play->addWidget(m_pStartBtn);
}

void DFMVaultActiveStartView::slotStartBtnClicked()
{
    emit sigAccepted();
}
