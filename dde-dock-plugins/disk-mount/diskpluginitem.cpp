#include "diskpluginitem.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QIcon>
#include <QtMath>

DiskPluginItem::DiskPluginItem(QWidget *parent)
    : QWidget(parent),
      m_displayMode(Dock::Efficient)
{
//    QIcon::setThemeName("deepin");
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
    painter.drawPixmap(rect().center() - m_icon.rect().center(), m_icon);
}

void DiskPluginItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateIcon();
}

void DiskPluginItem::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::RightButton)
        return QWidget::mousePressEvent(e);

    const QPoint p(e->pos() - rect().center());
    if (p.manhattanLength() < std::min(width(), height()) * 0.8 * 0.5)
    {
        emit requestContextMenu();
        return;
    }

    QWidget::mousePressEvent(e);
}

QSize DiskPluginItem::sizeHint() const
{
    return QSize(26, 26);
}

void DiskPluginItem::updateIcon()
{
    if (m_displayMode == Dock::Efficient)
//        m_icon = ImageUtil::loadSvg(":/icons/resources/icon-small.svg", 16);
        m_icon = QIcon::fromTheme("drive-removable-dock-symbolic").pixmap(16, 16);
    else
//        m_icon = ImageUtil::loadSvg(":/icons/resources/icon.svg", std::min(width(), height()) * 0.8);
        m_icon = QIcon::fromTheme("drive-removable-dock").pixmap(std::min(width(), height()) * 0.8, std::min(width(), height()) * 0.8);

    update();
}
