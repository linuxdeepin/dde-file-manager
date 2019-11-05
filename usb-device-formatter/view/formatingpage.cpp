/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "formatingpage.h"

#include <DWaterProgress>

#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE

FormatingPage::FormatingPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FormatingPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    m_progressBox = new DWaterProgress(this);
    m_progressBox->setFixedSize(128,128);
    m_progressBox->start();
    QLabel* formatingLabel = new QLabel(this);
    QString formatingText = tr("Formatting the disk, please wait...");
    formatingLabel->setText(formatingText);
    formatingLabel->setObjectName("StatusLabel");

    mainLayout->addSpacing(29);
    mainLayout->addWidget(m_progressBox, 0 , Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(formatingLabel, 0 , Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void FormatingPage::setProgress(double p)
{
    m_progressBox->setValue(int(p * 100));
}
