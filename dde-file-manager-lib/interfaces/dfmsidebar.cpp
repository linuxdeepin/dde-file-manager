/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dfmsidebar.h"
#include "views/dfmsidebaritemgroup.h"

#include <DThemeManager>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarPrivate
{
    Q_DECLARE_PUBLIC(DFMSideBar)

public:
    DFMSideBarPrivate(DFMSideBar *qq);

    DFMSideBar *q_ptr = nullptr;
    QVBoxLayout *mainLayout;

private:
    void initData();
    void initUI();
};

DFMSideBarPrivate::DFMSideBarPrivate(DFMSideBar *qq)
    : q_ptr(qq)
{
    initData();
    initUI();
}

void DFMSideBarPrivate::initData()
{

}

void DFMSideBarPrivate::initUI()
{
    Q_Q(DFMSideBar);
    q->setAcceptDrops(true);
    q->setFocusPolicy(Qt::NoFocus);

    // our main QVBoxLayout, which hold our `DFMSideBarItemGroup`s
    mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    q->setLayout(mainLayout);

    static QList<DFMSideBar::GroupName> groups = {
        DFMSideBar::GroupName::Common,
        DFMSideBar::GroupName::Device,
        DFMSideBar::GroupName::Bookmark,
        DFMSideBar::GroupName::Network,
        DFMSideBar::GroupName::Tag
    };

    foreach (const DFMSideBar::GroupName &groupType, groups) {
        mainLayout->addLayout(new DFMSideBarItemGroup(groupType));
    }
}

DFMSideBar::DFMSideBar(QWidget *parent)
    : QScrollArea(parent)
    , d_ptr(new DFMSideBarPrivate(this))
{
    DThemeManager::instance()->registerWidget(this);
}

DFMSideBar::~DFMSideBar()
{

}

void DFMSideBar::setDisableUrlSchemes(const QStringList &schemes)
{

}

QRect DFMSideBar::groupGeometry(const QString &group) const
{

}

DFMSideBar::GroupName DFMSideBar::groupFromName(const QString &name)
{
    if (name.isEmpty()) {
        return Other;
    }

    switch (name.toLatin1().at(0)) {
    case 'c':
        if (name == QStringLiteral("common")) {
            return Common;
        }

        break;
    case 'd':
        if (name == QStringLiteral("device")) {
            return Device;
        }

        break;
    case 'b':
        if (name == QStringLiteral("bookmark")) {
            return Bookmark;
        }

        break;
    case 'n':
        if (name == QStringLiteral("network")) {
            return Network;
        }

        break;
    case 't':
        if (name == QStringLiteral("tag")) {
            return Tag;
        }

        break;
    default:
        break;
    }

    return Unknow;
}

QString DFMSideBar::groupName(DFMSideBar::GroupName group)
{
    Q_ASSERT(group != Unknow);

    switch (group) {
    case Common:
        return "common";
    case Device:
        return "device";
    case Bookmark:
        return "bookmark";
    case Network:
        return "network";
    case Tag:
        return "tag";
    case Other: // deliberate
    default:
        break;
    }

    return QString();
}

DFM_END_NAMESPACE
