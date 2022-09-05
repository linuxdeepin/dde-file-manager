// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    // 修复bug-89675 切换模式时，diskpluginitem大小会变化，导致了绘制位置不对
    // 于是出现了切换模式时，图标不见了。
    // 保持diskpluginitem横纵比
    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    if (position == Dock::Bottom || position == Dock::Top) {
        setMaximumWidth(height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    } else {
        setMaximumHeight(width());
        setMaximumWidth(QWIDGETSIZE_MAX);
    }

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
