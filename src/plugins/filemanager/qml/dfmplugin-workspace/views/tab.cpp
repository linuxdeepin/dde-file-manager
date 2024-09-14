// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tab.h"
#include "private/tab_p.h"
#include "utils/filedatamanager.h"

#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-framework/event/event.h>

#include <DWidgetUtil>
#include <DPalette>
#include <dpalettehelper.h>
#include <dguiapplicationhelper.h>

#include <qdrawutil.h>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QDrag>
#include <QMimeData>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

Tab::Tab(QGraphicsObject *parent)
    : QGraphicsObject(parent),
      d(new TabPrivate())
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QUrl Tab::getCurrentUrl() const
{
    return d->url;
}

void Tab::setCurrentUrl(const QUrl &url)
{
    d->url = url;

    QString fileName = getDisplayNameByUrl(url);

    d->tabAlias.clear();
    dpfHookSequence->run("dfmplugin_workspace", "hook_Tab_SetTabName", url, &d->tabAlias);

    setTabText(fileName);
}

void Tab::setTabText(const QString &text)
{
    d->tabText = text;
    update();
}

void Tab::setTabAlias(const QString &alias)
{
    d->tabAlias = alias;
    update();
}

bool Tab::isChecked() const
{
    return d->checked;
}

void Tab::setChecked(const bool check)
{
    d->checked = check;
    update();
}

int Tab::width() const
{
    return d->width;
}

int Tab::height() const
{
    return d->height;
}

void Tab::setGeometry(const QRect &rect)
{
    prepareGeometryChange();
    setX(rect.x());
    setY(rect.y());
    d->width = rect.width();
    d->height = rect.height();
}

QRect Tab::geometry() const
{
    return QRect(static_cast<int>(x()), static_cast<int>(y()), d->width, d->height);
}

bool Tab::isDragging() const
{
    return d->isDragging;
}

void Tab::setHovered(bool hovered)
{
    d->hovered = hovered;
}

bool Tab::isDragOutSide()
{
    return d->dragOutSide;
}

bool Tab::borderLeft() const
{
    return d->borderLeft;
}

void Tab::setBorderLeft(const bool flag)
{
    d->borderLeft = flag;
}

QRectF Tab::boundingRect() const
{
    return QRectF(0, 0, d->width, d->height);
}

QPainterPath Tab::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Tab::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)

    if (d->dragOutSide)
        return;

    QPen pen = painter->pen();
    pen.setWidth(1);

    // draw text
    QFont font;
    font.setPixelSize(12);

    if (isChecked()) {
        font.setWeight(QFont::DemiBold);
    } else {
        font.setWeight(QFont::Normal);
    }

    painter->setFont(font);
    QFontMetrics fm(font);
    QString tabName = d->tabAlias.isEmpty() ? d->tabText : d->tabAlias;
    QString str = fm.elidedText(tabName, Qt::ElideRight, d->width - 10);

    DPalette pal = DPaletteHelper::instance()->palette(widget);
    QColor color;

    // draw backgound
    if (isChecked()) {
        color = pal.color(QPalette::Active, QPalette::Base);
        color = DGuiApplicationHelper::adjustColor(color, 0, 0, 0, 0, 0, 0, +51);
        painter->fillRect(boundingRect(), color);

        color = pal.color(QPalette::Active, QPalette::Highlight);
        QPen tPen = painter->pen();
        tPen.setColor(color);
        painter->setPen(tPen);
        painter->drawText((d->width - fm.horizontalAdvance(str)) / 2, (d->height - fm.height()) / 2,
                          fm.horizontalAdvance(str), fm.height(), 0, str);
    } else if (d->hovered || (d->hovered && !isChecked())) {
        color = pal.color(QPalette::Active, QPalette::Light);
        color = DGuiApplicationHelper::adjustColor(color, 0, 0, 0, +2, +2, +2, +51);
        painter->fillRect(boundingRect(), color);
        painter->drawText((d->width - fm.horizontalAdvance(str)) / 2, (d->height - fm.height()) / 2,
                          fm.horizontalAdvance(str), fm.height(), 0, str);
    } else {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            color = QColor(235, 235, 235, 204);
        else
            color = QColor(30, 30, 30, 204);
        painter->fillRect(boundingRect(), color);
        painter->drawText((d->width - fm.horizontalAdvance(str)) / 2, (d->height - fm.height()) / 2,
                          fm.horizontalAdvance(str), fm.height(), 0, str);
    }

    // draw line
    pen.setColor(pal.color(QPalette::Inactive, QPalette::Shadow));
    painter->setPen(pen);
    int y = static_cast<int>(boundingRect().height());
    int x = static_cast<int>(boundingRect().width());

    qDrawShadeLine(painter, QPoint(x, 0), QPoint(x, y), pal);
    QPalette::ColorGroup cp = isChecked() || d->hovered ? QPalette::Active : QPalette::Inactive;
    pen.setColor(pal.color(cp, QPalette::WindowText));
    painter->setPen(pen);
}

void Tab::onFileRootUrlChanged(const QUrl &url)
{
    setCurrentUrl(url);
}

void Tab::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->dragOutSide) {
        d->pressed = false;
        setZValue(1);
        QGraphicsObject::mouseReleaseEvent(event);

        d->dragOutSide = false;
        d->isDragging = false;
        return;
    }

    emit clicked();
    d->pressed = false;
    setZValue(1);
    d->isDragging = false;
    d->borderLeft = false;
    update();
    emit draggingFinished();

    QGraphicsObject::mouseReleaseEvent(event);
}

void Tab::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->isDragging && !d->borderLeft) {
        d->borderLeft = true;
        update();
    }

    if (event->pos().y() < -d->height || event->pos().y() > d->height * 2) {
        if (!d->dragOutSide) {
            d->dragOutSide = true;
            update();
            emit aboutToNewWindow(this);
            emit draggingFinished();
            d->dragObject = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            int radius = 20;

            const QPixmap &pixmap = toPixmap(true);
            QImage image = Dtk::Widget::dropShadow(pixmap, radius, QColor(0, 0, 0, static_cast<int>(0.2 * 255)));
            QPainter pa(&image);

            pa.drawPixmap(radius, radius, pixmap);

            d->dragObject->setPixmap(QPixmap::fromImage(image));
            d->dragObject->setMimeData(mimeData);
            d->dragObject->setHotSpot(QPoint(150 + radius, 12 + radius));
            d->dragObject->exec();
            d->dragObject->deleteLater();
            d->pressed = false;

            emit requestNewWindow(d->url);
        }
    }

    if (d->dragOutSide) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    if (pos().x() == 0.0
        && static_cast<int>(pos().x()) == static_cast<int>(scene()->width() - d->width)) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }
    setPos(x() + event->pos().x() - event->lastPos().x(), 0);
    emit draggingStarted();
    d->isDragging = true;
    if (pos().x() < 0)
        setX(0);
    else if (pos().x() > scene()->width() - d->width)
        setX(scene()->width() - d->width);

    if (pos().x() > d->originPos.x() + d->width / 2) {
        emit moveNext(this);
        d->originPos.setX(d->originPos.x() + d->width);
    } else if (pos().x() < d->originPos.x() - d->width / 2) {
        emit movePrevius(this);
        d->originPos.setX(d->originPos.x() - d->width);
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void Tab::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        d->pressed = true;
        d->originPos = pos();
        setZValue(3);
    }
    QGraphicsObject::mousePressEvent(event);
}

void Tab::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    d->hovered = true;
    QGraphicsObject::hoverEnterEvent(event);
}

void Tab::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    d->hovered = false;
    d->pressed = false;
    QGraphicsObject::hoverLeaveEvent(event);
}

QPixmap Tab::toPixmap(bool drawBorder) const
{
    QImage img(300, d->height, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    QPen pen;
    QColor color(Qt::yellow);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);

    // draw text
    QFont font;
    font.setPixelSize(12);
    painter.setFont(font);
    QFontMetrics fm(font);
    QString tabName = d->tabAlias.isEmpty() ? d->tabText : d->tabAlias;
    QString str = fm.elidedText(tabName, Qt::ElideRight, 300 - 10);

    // draw backgound
    color.setNamedColor("#FFFFFF");
    painter.fillRect(boundingRect(), color);
    color.setNamedColor("#303030");
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawText((300 - fm.horizontalAdvance(str)) / 2, (d->height - fm.height()) / 2,
                     fm.horizontalAdvance(str), fm.height(), 0, str);

    if (drawBorder) {
        QPainterPath path;
        path.addRect(0, 0, 300 - 1, d->height - 1);
        color.setRgb(0, 0, 0, static_cast<int>(0.1 * 255));
        pen.setColor(color);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    return QPixmap::fromImage(img);
}

QString Tab::getDisplayNameByUrl(const QUrl &url) const
{
    if (UrlRoute::isRootUrl(url))
        return UrlRoute::rootDisplayName(url.scheme());

    if (SystemPathUtil::instance()->isSystemPath(url.path()))
        return SystemPathUtil::instance()->systemPathDisplayNameByPath(url.path());

    if (url.fileName().isEmpty()) {
        auto info = InfoFactory::create<FileInfo>(url);
        return info->nameOf(DFMBASE_NAMESPACE::FileInfo::FileNameInfoType::kFileName);
    }

    return url.fileName();
}
