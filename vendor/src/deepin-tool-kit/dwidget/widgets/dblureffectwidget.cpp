#include "dblureffectwidget.h"
#include "private/dblureffectwidget_p.h"

#include <QPainter>
#include <QBackingStore>
#include <QPaintEvent>

#include <qpa/qplatformbackingstore.h>

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE

DBlurEffectWidgetPrivate::DBlurEffectWidgetPrivate(DBlurEffectWidget *qq)
    : DObjectPrivate(qq)
{

}

DBlurEffectWidget::DBlurEffectWidget(QWidget *parent)
    : QWidget(parent)
    , DObject(*new DBlurEffectWidgetPrivate(this))
{

}

int DBlurEffectWidget::radius() const
{
    D_DC(DBlurEffectWidget);

    return d->radius;
}

DBlurEffectWidget::BlurMode DBlurEffectWidget::mode() const
{
    D_DC(DBlurEffectWidget);

    return d->mode;
}

void DBlurEffectWidget::setRadius(int radius)
{
    D_D(DBlurEffectWidget);

    if (d->radius == radius)
        return;

    d->radius = radius;

    update();

    emit radiusChanged(radius);
}

void DBlurEffectWidget::setMode(DBlurEffectWidget::BlurMode mode)
{
    D_D(DBlurEffectWidget);

    if (d->mode == mode)
        return;

    d->mode = mode;

    emit modeChanged(mode);
}

DBlurEffectWidget::DBlurEffectWidget(DBlurEffectWidgetPrivate &dd, QWidget *parent)
    : QWidget(parent)
    , DObject(dd)
{

}

void DBlurEffectWidget::paintEvent(QPaintEvent *event)
{
    D_DC(DBlurEffectWidget);

    QPainter pa(this);

    QPoint point_offset = mapTo(window(), QPoint(0, 0));
    int radius = d->radius;

    for (const QRect &rect : event->region().rects()) {
        const QRect &tmp_rect = rect.translated(point_offset).adjusted(-radius, -radius, radius, radius);

        QImage image = window()->backingStore()->handle()->toImage().copy(tmp_rect);

        qt_blurImage(image, radius, false, false);

        pa.drawImage(rect.topLeft(), image, QRect(QPoint(radius, radius), rect.size()));
    }
}

DWIDGET_END_NAMESPACE
