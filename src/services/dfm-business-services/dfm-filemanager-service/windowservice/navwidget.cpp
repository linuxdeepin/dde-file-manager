/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "navwidget.h"

DSB_FM_BEGIN_NAMESPACE

namespace GlobalPrivate {
    static int listIdx = -1;
    static QList<QUrl> urlCacheList{};
} //namespace GlobalPrivate

DFMNavWidget::DFMNavWidget(QWidget *parent) : QWidget(parent)
{
    if (!m_navBackButton)
        m_navBackButton = new DButtonBoxButton(QStyle::SP_ArrowBack);
    if (!m_navForwardButton)
        m_navForwardButton = new DButtonBoxButton(QStyle::SP_ArrowForward);
    if (!m_buttonBox)
        m_buttonBox = new DButtonBox;
    if (!m_hboxLayout)
        m_hboxLayout = new QHBoxLayout;

    m_buttonBox->setButtonList({m_navBackButton,m_navForwardButton},false);
    this->setLayout(m_hboxLayout);
    m_hboxLayout->addWidget(m_buttonBox);

    m_hboxLayout->setSpacing(0);
    m_hboxLayout->setContentsMargins(0, 0, 0, 0);

    QObject::connect(m_navBackButton, &DButtonBoxButton::clicked,
                     this, &DFMNavWidget::doButtonClicked,
                     Qt::UniqueConnection);

    QObject::connect(m_navForwardButton, &DButtonBoxButton::clicked,
                     this, &DFMNavWidget::doButtonClicked,
                     Qt::UniqueConnection);
}

DButtonBoxButton *DFMNavWidget::navBackButton() const
{
    return m_navBackButton;
}

void DFMNavWidget::setNavBackButton(DButtonBoxButton *navBackButton)
{
    if (!navBackButton) return;

    if (m_navBackButton) {
        delete m_navBackButton;
        m_navBackButton = nullptr;
    }

    if (!m_navBackButton)
        m_navBackButton = navBackButton;

    if (m_navBackButton->icon().isNull())
        m_navBackButton->setIcon(QStyle::SP_ArrowBack);

    m_buttonBox->setButtonList({m_navBackButton,m_navForwardButton},false);
}

DButtonBoxButton *DFMNavWidget::navForwardButton() const
{
    return m_navForwardButton;
}

void DFMNavWidget::setNavForwardButton(DButtonBoxButton *navForwardButton)
{
    if (!navForwardButton) return;

    if (m_navForwardButton) {
        delete m_navForwardButton;
        m_navForwardButton = nullptr;
    }

    if(!m_navForwardButton)
        m_navForwardButton = navForwardButton;

    if (m_navBackButton->icon().isNull())
        m_navBackButton->setIcon(QStyle::SP_ArrowForward);

    m_buttonBox->setButtonList({m_navBackButton,m_navForwardButton},false);
}

void DFMNavWidget::doButtonClicked()
{
    if (!sender() || GlobalPrivate::listIdx == -1) return;

    if (sender() == m_navBackButton) {
        if (0 == GlobalPrivate::listIdx) {
            Q_EMIT releaseUrl(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx]);
            return;  //头节点
        }
        releaseUrl(GlobalPrivate::urlCacheList[-- GlobalPrivate::listIdx]);
        return;
    }

    if (sender() == m_navForwardButton) {
        if (GlobalPrivate::urlCacheList.size() - 1 == GlobalPrivate::listIdx ) {
            Q_EMIT releaseUrl(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx]); //尾节点
            return;
        }
        releaseUrl(GlobalPrivate::urlCacheList[++ GlobalPrivate::listIdx]);
        return;
    }
}

void DFMNavWidget::appendUrl(const QUrl &url)
{
    if (GlobalPrivate::listIdx != -1) {
        if(GlobalPrivate::urlCacheList[GlobalPrivate::listIdx] == url) return;//略过当前目录的重复点击
    }

    GlobalPrivate::urlCacheList.append(url);
    GlobalPrivate::listIdx = GlobalPrivate::urlCacheList.size() - 1; //始终保持指针指向最后
}

DSB_FM_END_NAMESPACE
