/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *
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

class QHBoxLayout;
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
private slots:
    void onChangeAuthorizationLabel(int);
private:
    QString m_configFile;
    QJsonObject m_configs;
    QLabel* m_logoLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    bool m_isMaskAlwaysOn = false;
    int m_xRightBottom;
    int m_yRightBottom;
    int m_maskWidth;
    int m_maskHeight;
};

#endif // WATERMASKFRAME_H
