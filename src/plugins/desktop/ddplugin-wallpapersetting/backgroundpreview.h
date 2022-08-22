/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef BACKGROUNDPREVIEW_H
#define BACKGROUNDPREVIEW_H

#include "ddplugin_wallpapersetting_global.h"

#include "interfaces/background/abstractbackground.h"

namespace ddplugin_wallpapersetting {

class BackgroundPreview : public DFMBASE_NAMESPACE::AbstractBackground
{
public:
    explicit BackgroundPreview(const QString &screenName, QWidget *parent = nullptr);
    virtual void setMode(int mode) override;
    virtual void setDisplay(const QString &path) override;
    virtual void updateDisplay() override;
protected:
    void paintEvent(QPaintEvent *event) override;
    QPixmap getPixmap(const QString &path, const QPixmap &defalutPixmap);

private:
    QPixmap pixmap;
    QPixmap noScalePixmap;
};

typedef QSharedPointer<BackgroundPreview> PreviewWidgetPtr;
}

#endif // BACKGROUNDPREVIEW_H
