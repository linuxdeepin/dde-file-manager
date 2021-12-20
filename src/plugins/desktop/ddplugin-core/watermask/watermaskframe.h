/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef WATERMASKFRAME_H
#define WATERMASKFRAME_H

#include "dfm_desktop_service_global.h"

#include <QFrame>

class QHBoxLayout;
class QLabel;
class QJsonObject;
DSB_D_BEGIN_NAMESPACE

class WaterMaskFrame : public QFrame
{
    Q_OBJECT
public:
    explicit WaterMaskFrame(const QString& fileName, QWidget *parent = nullptr);
    ~WaterMaskFrame();
public slots:
    void refresh();
    void updatePosition();
protected slots:
    void stateChanged(int);
protected:
    struct ConfigInfo
    {
        bool maskAlwaysOn = true;
        QString maskLogoUri;
        int maskLogoWidth = 128;
        int maskLogoHeight = 48;
        int maskTextWidth = 100;
        int maskTextHeight = 30;
        int maskWidth = maskLogoWidth + maskTextWidth;
        int maskHeight = 48;
        int maskLogoTextSpacing = 0;
        int xRightBottom = 50;
        int yRightBottom = 98;
    };
    static ConfigInfo parseJson(QJsonObject *);
    static QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
protected:
    static bool showLicenseState();
    static void addWidget(QHBoxLayout *layout, QWidget *wid, const QString &align);
    void loadConfig();
    void update(const ConfigInfo &);
    void setTextAlign(const QString &maskTextAlign);
private:
    QString configFile;
    ConfigInfo currentConfig;
    QLabel *logoLabel = nullptr;
    QLabel *textLabel = nullptr;
};

DSB_D_END_NAMESPACE

#endif // WATERMASKFRAME_H
