/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef WATERMASKFRAME_H
#define WATERMASKFRAME_H

#include "dbus/licenceInterface.h"

#include <QFrame>
#include <QJsonObject>
#include <QLabel>

enum ActiveState {
    Unauthorized = 0,  //未授权
    Authorized,  //已授权
    AuthorizedLapse,  //授权失效
    TrialAuthorized, //试用期已授权
    TrialExpired //试用期已过期
};

class WaterMaskFrame : public QFrame
{
    Q_OBJECT
public:
    explicit WaterMaskFrame(const QString& fileName, QWidget *parent = nullptr);
    ~WaterMaskFrame();
    bool checkConfigFile(const QString& fileName);
    void loadConfig(const QString& fileName);
    void initUI();

private:
    bool isNeedState();
    bool parseJson(QString key);

public slots:
    void updatePosition();
    void updateAuthorizationState();

private:
    QString m_configFile;
    QJsonObject m_configs;
    QLabel* m_logoLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    bool m_isMaskAlwaysOn = false;
    int m_xRightBottom;
    int m_yRightBottom;
    int m_maskWidth;
    int m_maskHeight;

    std::unique_ptr<ComDeepinLicenseInterface> m_licenseInterface;
};

#endif // WATERMASKFRAME_H
