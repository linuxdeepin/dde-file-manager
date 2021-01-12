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

#include "diskpluginitem.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

DiskPluginItem::DiskPluginItem(QWidget *parent)
    : QWidget(parent),
      m_displayMode(Dock::Efficient)
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        updateIcon();
    });
}

void DiskPluginItem::setDockDisplayMode(const Dock::DisplayMode mode)
{
    m_displayMode = mode;

    updateIcon();
}

void DiskPluginItem::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(m_icon.rect());
    painter.drawPixmap(rf.center() - rfp.center() / qApp->devicePixelRatio(), m_icon);
}

void DiskPluginItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateIcon();
}

QSize DiskPluginItem::sizeHint() const
{
    return QSize(26, 26);
}

void DiskPluginItem::updateIcon()
{
//    if (m_displayMode == Dock::Efficient)
//        m_icon = QIcon::fromTheme("drive-removable-dock-symbolic").pixmap(16 * qApp->devicePixelRatio(), 16 * qApp->devicePixelRatio());
//    else
//        m_icon = QIcon::fromTheme("drive-removable-dock").pixmap(std::min(width(), height()) * 0.8 * qApp->devicePixelRatio(), std::min(width(), height()) * 0.8 * qApp->devicePixelRatio());

    // fashion mode icons are no longer needed

    QString iconName = "drive-removable-dock-symbolic";
    if (height() <= PLUGIN_BACKGROUND_MIN_SIZE && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        iconName.append(PLUGIN_MIN_ICON_NAME);

    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/icons/resources/%1.svg").arg(iconName))).pixmap(static_cast<int>(16 * qApp->devicePixelRatio()), static_cast<int>(16 * qApp->devicePixelRatio()));
    m_icon.setDevicePixelRatio(qApp->devicePixelRatio());
    update();
}
