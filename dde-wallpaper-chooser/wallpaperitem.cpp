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

#include "wallpaperitem.h"
#include "constants.h"
#include "thumbnailmanager.h"
#include "button.h"

#include <QEvent>
#include <QPixmap>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QApplication>

class WrapperWidget : public QWidget
{
public:
    explicit WrapperWidget(QWidget *parent = 0) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        if (m_pixmap.isNull()) return;

        QRect pixmap_geometry = m_pixmapBoxGeometry;

        pixmap_geometry.setSize(m_pixmap.size());
        pixmap_geometry.moveCenter(m_pixmapBoxGeometry.center());

        QPainter pa(this);

        pa.setOpacity(m_opacity);
        pa.drawPixmap(pixmap_geometry.topLeft(), m_pixmap, pixmap_geometry.united(event->rect()));
        pa.setPen(QColor(255, 255, 255, 25));
        pa.drawRect(m_pixmapBoxGeometry.adjusted(0, 0, -1, -1));
    }

private:
    QPixmap m_pixmap;
    qreal m_opacity = 1;
    QRect m_pixmapBoxGeometry;

    friend class WallpaperItem;
};

WallpaperItem::WallpaperItem(QFrame *parent, const QString &path) :
    QFrame(parent),
    m_path(path),
    m_thumbnailerWatcher(new QFutureWatcher<QPixmap>(this))
{
    initUI();
    initAnimation();

//    connect(m_thumbnailerWatcher, &QFutureWatcher<QPixmap>::finished, this, &WallpaperItem::thumbnailFinished);
}

WallpaperItem::~WallpaperItem()
{

}

void WallpaperItem::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);

    m_wrapper = new WrapperWidget(this);
    m_wrapper->setFixedSize(ItemWidth, ItemHeight * 2);
    m_wrapper->setAttribute(Qt::WA_TranslucentBackground);

    QFrame * buttonArea = new QFrame(m_wrapper);
    buttonArea->setFixedSize(ItemWidth, ItemHeight);
    buttonArea->move(0, ItemHeight);

    m_buttonLayout = new QVBoxLayout(buttonArea);
    m_buttonLayout->setSpacing(10);
    m_buttonLayout->setContentsMargins(0, 10, 0, 10);
}

void WallpaperItem::initAnimation()
{
    m_upAnim = new QPropertyAnimation(m_wrapper, "pos");
    m_upAnim->setDuration(300);

    m_downAnim = new QPropertyAnimation(m_wrapper, "pos");
    m_downAnim->setDuration(300);
}

void WallpaperItem::initPixmap()
{
    if (m_useThumbnailManager) {
        refindPixmap();
    } else {
        QIcon icon(m_path);
        m_wrapper->m_pixmap = icon.pixmap(window()->windowHandle(), QSize(ItemWidth, ItemHeight));
        // 由于QIcon::pixmap中计算图片缩放比例时的特殊计算方式，当请求的pixmap大小的比例和图片实际大小比例不一致时
        // 计算出的pixmap缩放系数和希望的值不一致，所以此处纠正一下
        m_wrapper->m_pixmap.setDevicePixelRatio(qMax(m_wrapper->m_pixmap.width() / qreal(ItemWidth),
                                                     m_wrapper->m_pixmap.height() / qreal(ItemHeight)));
        m_wrapper->update();
    }
}

QString WallpaperItem::data() const
{
    return m_data;
}

bool WallpaperItem::useThumbnailManager() const
{
    return m_useThumbnailManager;
}

QPushButton *WallpaperItem::addButton(const QString &id, const QString &text)
{
    Button *button = new Button(this);
    button->setText(text);
    button->setAttract(false);

    connect(button, &Button::clicked, this, [this, id] {
        emit buttonClicked(id);
    });

    m_buttonLayout->addWidget(button, 0, Qt::AlignHCenter | Qt::AlignTop);

    return button;
}

void WallpaperItem::slideUp()
{
    if (m_wrapper->y() < 0)
        return;

    m_upAnim->setStartValue(QPoint(0, 0));
    m_upAnim->setEndValue(QPoint(0, -ItemHeight / 2 * m_buttonLayout->count()));
    m_upAnim->start();
}

void WallpaperItem::slideDown()
{
    if (m_wrapper->y() >= 0)
        return;

    m_downAnim->setStartValue(QPoint(0, -ItemHeight / 2 * m_buttonLayout->count()));
    m_downAnim->setEndValue(QPoint(0, 0));
    m_downAnim->start();
}

QString WallpaperItem::getPath() const
{
    if (m_path.startsWith("/"))
        return m_path;

    QUrl url = QUrl::fromPercentEncoding(m_path.toUtf8());
    return url.toLocalFile();
}

void WallpaperItem::thumbnailFinished()
{
    QFuture<QPixmap> f = m_thumbnailerWatcher->future();
    m_wrapper->m_pixmap = f.result();
    m_wrapper->update();
}

void WallpaperItem::setData(const QString &data)
{
    m_data = data;
}

void WallpaperItem::setUseThumbnailManager(bool useThumbnailManager)
{
    m_useThumbnailManager = useThumbnailManager;
}

void WallpaperItem::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
        emit pressed();
}

void WallpaperItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    emit hoverIn();
}

void WallpaperItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    emit hoverOut();
}

void WallpaperItem::resizeEvent(QResizeEvent *event)
{
    const qreal ratio = devicePixelRatioF();

    const QPoint &offset = QPoint((event->size().width() - ItemWidth) / 2,
                                  (event->size().height() - ItemHeight) / 2);

    m_wrapper->setFixedWidth(width());
    m_wrapper->m_pixmapBoxGeometry = QRect(offset * ratio, QSize(ItemWidth * ratio, ItemHeight * ratio));

    QFrame::resizeEvent(event);
}

void WallpaperItem::refindPixmap()
{
    ThumbnailManager *tnm = ThumbnailManager::instance(devicePixelRatioF());

    connect(tnm, &ThumbnailManager::thumbnailFounded, this, &WallpaperItem::onThumbnailFounded, Qt::UniqueConnection);
    connect(tnm, &ThumbnailManager::findAborted, this, &WallpaperItem::onFindAborted, Qt::UniqueConnection);

    tnm->find(QUrl::toPercentEncoding(m_path));
}

void WallpaperItem::onFindAborted(const QQueue<QString> &list)
{
    if (list.contains(QUrl::toPercentEncoding(m_path))) {
        refindPixmap();
    }
}

bool WallpaperItem::getDeletable() const
{
    return m_deletable;
}

void WallpaperItem::setDeletable(bool deletable)
{
    m_deletable = deletable;
}

void WallpaperItem::setOpacity(qreal opacity)
{
    if (m_wrapper->m_opacity == opacity)
        return;

    m_wrapper->m_opacity = opacity;
    m_wrapper->update();
}

QRect WallpaperItem::contentImageGeometry() const
{
    return m_wrapper->m_pixmapBoxGeometry;
}

void WallpaperItem::onThumbnailFounded(const QString &key, const QPixmap &pixmap)
{
    if (key != QUrl::toPercentEncoding(m_path))
        return;

    const qreal ratio = devicePixelRatioF();

    m_wrapper->m_pixmap = pixmap;
    m_wrapper->m_pixmap.setDevicePixelRatio(ratio);
    m_wrapper->update();
}

void WallpaperItem::setPath(const QString &path)
{
    m_path = path;
}
