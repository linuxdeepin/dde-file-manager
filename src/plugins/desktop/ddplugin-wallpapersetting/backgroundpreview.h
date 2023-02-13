// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
