/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include "backgroundwidget.h"
#include "screen/abstractscreen.h"

#include <com_deepin_wm.h>
#include <DWindowManagerHelper>
#include <dgiosettings.h>

#include <QObject>
#include <QMap>

using WMInter = com::deepin::wm;

DGUI_USE_NAMESPACE
class AbstractScreen;
class BackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManager(bool preview = false,QObject *parent = nullptr);
    ~BackgroundManager();
    bool isEnabled() const;
    void setVisible(bool visible);
    bool isVisible() const;
    BackgroundWidgetPointer backgroundWidget(ScreenPointer) const;
    inline QMap<ScreenPointer,BackgroundWidgetPointer> allbackgroundWidgets() const{return m_backgroundMap;}
    void setBackgroundImage(const QString &screen,const QString &path);
    inline QMap<QString,QString> backgroundImages() const {return m_backgroundImagePath;}
signals:
    void sigBackgroundBuilded(int mode); //通知canvasview
public slots:
    void onBackgroundBuild();       //创建背景窗口
    void onSkipBackgroundBuild();   //不创建背景窗口，直接发完成信号
    void onResetBackgroundImage();
    void onWmDbusStarted(QString name, QString oldOwner, QString newOwner); //窗管dbus服务启动完成
protected slots:
    void onRestBackgroundManager(); //重置背景，响应窗管改变
    void onScreenGeometryChanged();    //响应屏幕大小改变
private:
    void init();
    void pullImageSettings();
    QString getBackgroundFromWm(const QString &screen);
    QString getBackgroundFromWmConfig(const QString &screen);
    QString getDefaultBackground() const;
    BackgroundWidgetPointer createBackgroundWidget(ScreenPointer);
protected:
    DGioSettings *gsettings = nullptr;
    WMInter *wmInter = nullptr;
    DWindowManagerHelper* windowManagerHelper = nullptr;
private:
    bool m_preview = false; //壁纸预览
    bool m_visible = true;
    int currentWorkspaceIndex = 1;
    bool m_backgroundEnable = true;

    QMap<ScreenPointer,BackgroundWidgetPointer> m_backgroundMap;

    //记录设置的背景的壁纸
    QMap<QString, QString> m_backgroundImagePath;
};

#endif // BACKGROUNDMANAGER_H
