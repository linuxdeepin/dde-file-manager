#include "backgroundwidget.h"
#include "canvasgridview.h"

#include <DSysInfo>
#include <QPaintEvent>
#include <QBackingStore>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>
#include <QPaintDevice>
#include <QPainter>
#include <QImage>
#include <qdebug.h>

BackgroundWidget::BackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    // 如果开启--video-wallpaper，桌面背景为透明
    if (Dtk::Core::DSysInfo::isCommunityEdition() && qApp->property("video-window-id").isValid()) {
        this->setAttribute(Qt::WA_TranslucentBackground, true);
    }

}

BackgroundWidget::~BackgroundWidget()
{
    if (m_view.get() != nullptr){
        m_view->setParent(nullptr);
        m_view = nullptr;
    }
}

void BackgroundWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_noScalePixmap = pixmap;
    m_noScalePixmap.setDevicePixelRatio(1);
    update();
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    // 如果开启--video-wallpaper，桌面背景为透明
    if (Dtk::Core::DSysInfo::isCommunityEdition() && qApp->property("video-window-id").isValid()) {
        QPainter pa(this);
        pa.fillRect(event->rect(), QBrush(Qt::transparent));
        return;
    }

    qreal scale = devicePixelRatioF();
    if (scale > 1.0 && event->rect() == rect()) {
        if (backingStore()->handle()->paintDevice()->devType() != QInternal::Image) {
            return;
        }

        QImage *image = static_cast<QImage *>(backingStore()->handle()->paintDevice());
        QPainter pa(image);
        pa.drawPixmap(0, 0, m_noScalePixmap);
        return;
    }

    QPainter pa(this);
    pa.drawPixmap(event->rect().topLeft(), m_pixmap, QRectF(QPointF(event->rect().topLeft()) * scale, QSizeF(event->rect().size()) * scale));
}

void BackgroundWidget::setView(const QSharedPointer<CanvasGridView> &v)
{
    v->setParent(this);
    m_view = v;
}
