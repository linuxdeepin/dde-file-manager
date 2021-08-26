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
#include "dfmnavwidget.h"

static int _listIdx = -1;
static QList<QUrl> m_urlCacheList{};

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
    if (!sender() || _listIdx == -1) return;

    if (sender() == m_navBackButton) {
        if (0 == _listIdx) {
            Q_EMIT releaseUrl(m_urlCacheList[_listIdx]);
            return;  //头节点
        }
        releaseUrl(m_urlCacheList[-- _listIdx]);
        return;
    }

    if (sender() == m_navForwardButton) {
        if (m_urlCacheList.size() - 1 == _listIdx ) {
            Q_EMIT releaseUrl(m_urlCacheList[_listIdx]); //尾节点
            return;
        }
        releaseUrl(m_urlCacheList[++ _listIdx]);
        return;
    }
}

void DFMNavWidget::appendUrl(const QUrl &url)
{
    if (_listIdx != -1) {
        if(m_urlCacheList[_listIdx] == url) return;//略过当前目录的重复点击
    }

    m_urlCacheList.append(url);
    _listIdx = m_urlCacheList.size() - 1; //始终保持指针指向最后
}
