#include "videowidget.h"
#include "videopreview.h"

#include <player_engine.h>

DWIDGET_USE_NAMESPACE
VideoWidget::VideoWidget(VideoPreview *preview)
   : dmr::PlayerWidget(nullptr)
   , p(preview)
   , title(new QLabel(this))
{
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   QPalette pa;
   pa.setColor(QPalette::Foreground, Qt::white);

   title->setPalette(pa);

   DAnchorsBase::setAnchor(title, Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);

   engine().setBackendProperty("keep-open", "yes");
}

QSize VideoWidget::sizeHint() const
{
   QSize screen_size;

   if (window()->windowHandle()) {
       screen_size = window()->windowHandle()->screen()->availableSize();
   } else {
       screen_size = qApp->desktop()->size();
   }

   return QSize(p->info.width, p->info.height).scaled(qMin(p->info.width, int(screen_size.width() * 0.5)),
                                                      qMin(p->info.height, int(screen_size.height() * 0.5)), Qt::KeepAspectRatio);
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
   p->pause();

   dmr::PlayerWidget::mouseReleaseEvent(event);
}
