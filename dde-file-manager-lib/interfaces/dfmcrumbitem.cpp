/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "dfmcrumbitem.h"
#include "dfmcrumbinterface.h"

#include "views/themeconfig.h"

#include <QPainter>
#include <DThemeManager>

#include <QDebug>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMCrumbItemPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbItem)

public:
    DFMCrumbItemPrivate(DFMCrumbItem *qq);

    QPixmap icon() const;
    ThemeConfig::State getState() const;

    QPoint clickedPos;
    CrumbData data;

    DFMCrumbItem *q_ptr = nullptr;

private:

};

DFMCrumbItemPrivate::DFMCrumbItemPrivate(DFMCrumbItem *qq)
    : q_ptr(qq)
{
    qq->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    qq->setObjectName("DCrumbButton");

    qq->connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, qq, [this, qq](){
        qq->setIcon(icon());
    });
}

QPixmap DFMCrumbItemPrivate::icon() const
{
    return ThemeConfig::instace()->pixmap(data.iconName, data.iconKey, getState());
}

ThemeConfig::State DFMCrumbItemPrivate::getState() const
{
    Q_Q(const DFMCrumbItem);

    if (q->isChecked()) {
        return ThemeConfig::Checked;
    }

    return ThemeConfig::Normal;
}


DFMCrumbItem::DFMCrumbItem(DUrl url, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    Q_UNUSED(url);
    //this->setStyleSheet("background: red");
    this->setText("test");
    this->setIconFromThemeConfig("CrumbIconButton.Home");
}

DFMCrumbItem::DFMCrumbItem(CrumbData data, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    if (!data.displayText.isEmpty()) {
        this->setText(data.displayText);
    }

    if (!data.iconName.isEmpty()) {
        this->setIconFromThemeConfig(data.iconName, data.iconKey);
    }
}

DFMCrumbItem::~DFMCrumbItem()
{

}

void DFMCrumbItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMCrumbItem);

    d->data.iconName = group;
    d->data.iconKey = key;

    // Do widget UI update.
    this->setIcon(d->icon());
}

void DFMCrumbItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);
    d->clickedPos = event->globalPos();

    return;
}

void DFMCrumbItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);

    if (d->clickedPos == event->globalPos() && !d->data.url.isEmpty()) {
        emit crumbItemClicked(d->data.url);
    }

    QWidget::mouseReleaseEvent(event);
}

void DFMCrumbItem::paintEvent(QPaintEvent *event)
{
    QPainter pa(this);

    //pa.fillRect(rect(), Qt::blue);

    QPushButton::paintEvent(event);
}

DFM_END_NAMESPACE
