/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef BACKGROUNDHELPER_H
#define BACKGROUNDHELPER_H

#include <QLabel>

#include <com_deepin_wm.h>
#include <DWindowManagerHelper>
#include <dgiosettings.h>

using WMInter = com::deepin::wm;

DGUI_USE_NAMESPACE


class BackgroundLabel : public QWidget
{
public:
    using QWidget::QWidget;

    void setPixmap(const QPixmap &pixmap);

    void paintEvent(QPaintEvent *event) override;

    virtual void setVisible(bool visible) override;

private:
    QPixmap m_pixmap;
    QPixmap m_noScalePixmap;
};

class DBusMonitor;
class BackgroundHelper : public QObject
{
    Q_OBJECT
    friend class CanvasGridView;
public:
    explicit BackgroundHelper(bool preview = false, QObject *parent = nullptr);
    ~BackgroundHelper();

    bool isEnabled() const;
    QWidget *waylandBackground(const QString &name) const;
    QList<QWidget*> waylandAllBackgrounds() const;
    QWidget *backgroundForScreen(QScreen *screen) const;
    QList<QWidget*> allBackgrounds() const;
    bool visible() const;

    void setBackground(const QString &path);
    QString background() const {return currentWallpaper;}
    void setVisible(bool visible);
    void resetBackgroundVisibleState();

public: //for debug
    void printLog();
    void printLog(int index);
    void mapLabelScreen(int labelIndex, int screenIndex);

Q_SIGNALS:
    void aboutDestoryBackground(QWidget *l);
    void enableChanged();
    void backgroundGeometryChanged(QWidget *l);
    void backgroundAdded(QWidget *l);

private:
    bool isKWin() const;
    bool isDeepinWM() const;
    void onWMChanged();
    void updateBackground(QWidget *l);
    void updateBackground();
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void updateBackgroundGeometry(QScreen *screen, BackgroundLabel * l);
    void checkBlackScreen();

    bool m_previuew = false;
    bool m_visible = true;

    DGioSettings          *gsettings          = nullptr;
    WMInter             *wmInter            = nullptr;
    DWindowManagerHelper* windowManagerHelper = nullptr;
    QTimer *checkTimer = nullptr;
    int currentWorkspaceIndex = 0;
    QString currentWallpaper;
    QPixmap backgroundPixmap;
    QMap<QScreen*, BackgroundLabel*> backgroundMap;

    QMap<QString,BackgroundLabel*> waylandbackgroundMap; //屏幕名称，背景窗
    QMap<QString,DBusMonitor *> waylandScreen;      //屏幕信息

    static BackgroundHelper *desktop_instance;

public:
    static BackgroundHelper* getDesktopInstance();

public slots:
    void monitorRectChanged();
};

#endif // BACKGROUNDHELPER_H
