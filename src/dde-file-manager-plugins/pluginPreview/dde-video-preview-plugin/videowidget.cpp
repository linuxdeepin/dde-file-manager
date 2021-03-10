/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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
