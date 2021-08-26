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
#include "dfmoptionbuttonbox.h"

QToolButton *DFMOptionButtonBox::detailButton() const
{
    return m_detailButton;
}

void DFMOptionButtonBox::setDetailButton(QToolButton *detailButton)
{
    if (!detailButton) return;

    if (!m_hBoxLayout->replaceWidget(detailButton, m_detailButton)->isEmpty()){

        if (m_detailButton) {
            delete m_detailButton;
            m_detailButton = nullptr;
        }

        if (!m_detailButton)
            m_detailButton = detailButton;

        if (m_listViewButton->icon().isNull())
            m_detailButton->setIcon(QIcon::fromTheme("dfm_rightview_detail"));

        m_detailButton->setCheckable(true);
        m_detailButton->setFocusPolicy(Qt::NoFocus);
        m_detailButton->setIconSize({16,16});
        m_detailButton->setFixedSize(36, 36);
    }
}

QToolButton *DFMOptionButtonBox::listViewButton() const
{
    return m_listViewButton;
}

void DFMOptionButtonBox::setListViewButton(QToolButton *listViewButton)
{
    if (!listViewButton) return;

    if (!m_hBoxLayout->replaceWidget(listViewButton, m_listViewButton)->isEmpty()){

        if (m_listViewButton) {
            delete m_listViewButton;
            m_listViewButton = nullptr;
        }

        if (!m_listViewButton)
            m_listViewButton = listViewButton;

        if (m_listViewButton->icon().isNull())
            m_listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));

        m_listViewButton->setCheckable(true);
        m_listViewButton->setFocusPolicy(Qt::NoFocus);
        m_listViewButton->setIconSize({16,16});
        m_listViewButton->setFixedSize(36, 36);
    }
}

DFMOptionButtonBox::DFMOptionButtonBox(QWidget *parent)
    : QWidget (parent)
{
    if (!m_iconViewButton)
        m_iconViewButton = new QToolButton;
    m_iconViewButton->setCheckable(true);
    m_iconViewButton->setFocusPolicy(Qt::NoFocus);
    m_iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));
    m_iconViewButton->setIconSize({16,16});
    m_iconViewButton->setFixedSize(36, 36);

    if (!m_listViewButton)
        m_listViewButton = new QToolButton;
    m_listViewButton->setCheckable(true);
    m_listViewButton->setFocusPolicy(Qt::NoFocus);
    m_listViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_details"));
    m_listViewButton->setIconSize({16,16});
    m_listViewButton->setFixedSize(36, 36);

    if (!m_detailButton)
        m_detailButton = new QToolButton;
    m_detailButton->setCheckable(true);
    m_detailButton->setFocusPolicy(Qt::NoFocus);
    m_detailButton->setIcon(QIcon::fromTheme("dfm_rightview_detail"));
    m_detailButton->setIconSize({16,16});
    m_detailButton->setFixedSize(36, 36);

    if (!m_hBoxLayout)
        m_hBoxLayout = new QHBoxLayout;
    m_hBoxLayout->addSpacing(15);
    m_hBoxLayout->addWidget(m_iconViewButton);
    m_hBoxLayout->addWidget(m_listViewButton);
    m_hBoxLayout->addWidget(m_detailButton);
    m_hBoxLayout->setContentsMargins(0,0,4,0);
    m_hBoxLayout->setSpacing(18);

    setLayout(m_hBoxLayout);
}

QToolButton *DFMOptionButtonBox::iconViewButton() const
{
    return m_iconViewButton;
}

void DFMOptionButtonBox::setIconViewButton(QToolButton *iconViewButton)
{
    if (!iconViewButton) return;

    if (!m_hBoxLayout->replaceWidget(iconViewButton, m_iconViewButton)->isEmpty()){

        if (m_iconViewButton) {
            delete m_iconViewButton;
            m_iconViewButton = nullptr;
        }

        if (!m_iconViewButton)
            m_iconViewButton = iconViewButton;

        if (m_iconViewButton->icon().isNull())
            m_iconViewButton->setIcon(QIcon::fromTheme("dfm_viewlist_icons"));

        m_iconViewButton->setCheckable(true);
        m_iconViewButton->setFocusPolicy(Qt::NoFocus);
        m_iconViewButton->setIconSize({16,16});
        m_iconViewButton->setFixedSize(36, 36);
    }
}
