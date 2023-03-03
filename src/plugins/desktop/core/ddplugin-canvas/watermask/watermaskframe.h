// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WATERMASKFRAME_H
#define WATERMASKFRAME_H

#include "ddplugin_canvas_global.h"

#include <QFrame>
#include <QMap>

class QHBoxLayout;
class QLabel;
class QJsonObject;
namespace ddplugin_canvas {

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
    void stateChanged(int state, int prop);
protected:
    struct ConfigInfo
    {
        bool valid = false;
        QString maskLogoUri;
        int maskLogoWidth = 232;
        int maskLogoHeight = 46;
        int maskTextWidth = 100;
        int maskTextHeight = 30;
        int maskWidth = maskLogoWidth + maskTextWidth;
        int maskHeight = 46;
        int maskLogoTextSpacing = 0;
        int xRightBottom = 60;
        int yRightBottom = 98;
    };
    QMap<QString, ConfigInfo> parseJson(QJsonObject *);
    static QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
protected:
    static bool showLicenseState();
    static void addWidget(QHBoxLayout *layout, QWidget *wid, const QString &align);
    static bool usingCn();
    void loadConfig();
    void update(const ConfigInfo &, bool normal);
    void setTextAlign(const QString &maskTextAlign);
private:
    ConfigInfo defaultCfg(QJsonObject *);
    ConfigInfo govCfg(QJsonObject *, bool cn);
    ConfigInfo entCfg(QJsonObject *, bool cn);
private:
    QString configFile;
    QMap<QString, ConfigInfo> configInfos;
    QLabel *logoLabel = nullptr;
    QLabel *textLabel = nullptr;
    bool maskAlwaysOn = true;
    int curState = -1;
    int curProperty = -1;
    QSize curMaskSize = QSize(0, 0);
    QPoint curRightBottom = QPoint(0, 0);
};

}
#endif // WATERMASKFRAME_H
