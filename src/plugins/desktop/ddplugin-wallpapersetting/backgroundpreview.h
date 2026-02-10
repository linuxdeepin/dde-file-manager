// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDPREVIEW_H
#define BACKGROUNDPREVIEW_H

#include "ddplugin_wallpapersetting_global.h"

#include <QWidget>

namespace ddplugin_wallpapersetting {

class BackgroundPreview : public QWidget
{
public:
    explicit BackgroundPreview(const QString &screenName, QWidget *parent = nullptr);
    void setDisplay(const QString &path);
    void updateDisplay();
protected:
    void paintEvent(QPaintEvent *event) override;
    QPixmap getPixmap(const QString &path, const QPixmap &defalutPixmap);

private:
    QString screen;
    QString filePath;
    QPixmap pixmap;
    QPixmap noScalePixmap;
};

typedef QSharedPointer<BackgroundPreview> PreviewWidgetPtr;
}

#endif // BACKGROUNDPREVIEW_H
