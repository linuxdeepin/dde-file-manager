// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskpluginitem.h"
#include "diskmountplugin.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <DGuiApplicationHelper>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

DGUI_USE_NAMESPACE

/*!
 * \class DiskPluginItem
 *
 * \brief DiskPluginItem is DiskMountPlugin::itemWidget
 * Return to the plugin master for display on the dde-dock panel
 */

DiskPluginItem::DiskPluginItem(QWidget *parent)
    : QWidget(parent),
      displayMode(Dock::Efficient)
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this] {
        updateIcon();
    });
}

void DiskPluginItem::setDockDisplayMode(const Dock::DisplayMode mode)
{
    displayMode = mode;
    updateIcon();
}

void DiskPluginItem::updateIcon()
{
    QString &&iconName = "drive-removable-dock-symbolic";

#if defined(USE_DOCK_NEW_INTERFACE)
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        iconName.append(PLUGIN_MIN_ICON_NAME);
#else
    // defines from `constan.h`
    if (height() <= PLUGIN_BACKGROUND_MIN_SIZE && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        iconName.append(PLUGIN_MIN_ICON_NAME);
#endif

    static const int kWidth = PLUGIN_BACKGROUND_MIN_SIZE;
    icon = QIcon::fromTheme(iconName, QIcon(QString(":/icons/resources/%1.svg").arg(iconName))).pixmap(kWidth, kWidth);
    icon.setDevicePixelRatio(qApp->devicePixelRatio());
    update();
}

QIcon DiskPluginItem::getIcon()
{
    return QIcon(icon);
}

void DiskPluginItem::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    QRectF &&rf = QRectF(rect());
    QRectF &&rfp = QRectF(icon.rect());
    painter.drawPixmap(rf.center() - rfp.center() / qApp->devicePixelRatio(), icon);
}

void DiskPluginItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    // bug-89675
    // The item size changed when switch dock style,
    // caused the drawing position to be abnormal and the icon to disappear
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
