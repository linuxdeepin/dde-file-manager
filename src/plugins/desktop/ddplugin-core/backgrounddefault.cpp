#include "backgrounddefault.h"
#include <QPaintEvent>
#include <QBackingStore>
#include <QPainter>
#include <QFileInfo>

BackgroundDefault::BackgroundDefault(QWidget *parent)
    : dfmbase::AbstractBackground (parent)
{

}

void BackgroundDefault::paintEvent(QPaintEvent *event)
{
    if (display().isEmpty()
            || !QFileInfo(display()).exists()
            || !QFileInfo(display()).isFile()) {
        return dfmbase::AbstractBackground::paintEvent(event);
    }

    if (Zoom::Fit == zoom()){
        QPainter painter(this);
        painter.drawPixmap(0,0,width(),height(),QPixmap::fromImage(QImage(display())));
        return;
    }
    // 这里写其他扩展逻辑
    return dfmbase::AbstractBackground::paintEvent(event);
}
