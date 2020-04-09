#include "backgroundwidget.h"

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
    pa.drawPixmap(event->rect().topLeft(), m_pixmap, QRect(event->rect().topLeft() * scale, event->rect().size() * scale));
}

void BackgroundWidget::setVisible(bool visible)
{
#if 0 //old
    if (!visible && !property("isPreview").toBool()) {
        // 暂时（紧急）解决arm64双屏切换复制模式容易出现无法显示桌面的问题，禁止隐藏任何桌面。
        // 后续有较好的解决方案可以删除此代码
        qDebug() << "not allow to hide desktop(screen is " << property("myScreen").toString() <<
                 ") primaryScreen is ";
        return ;
    }
#endif
    QWidget::setVisible(visible);
}
