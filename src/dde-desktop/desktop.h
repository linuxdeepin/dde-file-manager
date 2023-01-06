// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QObject>
#include <QScopedPointer>

#include "global/singleton.h"

#define DesktopServiceName          "com.deepin.dde.desktop"
#define DesktopServicePath          "/com/deepin/dde/desktop"
#define DesktopServiceInterface     "com.deepin.dde.desktop"

class QDBusConnection;
class CanvasGridView;
class DesktopPrivate;
class Desktop : public QObject, public DDEDesktop::Singleton<Desktop>
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DesktopServiceInterface)
public:
    void preInit();
    void loadData();
    void loadView();
    void showZoneSettings();

public slots:
    void ShowWallpaperChooser(const QString &screen = QString());
    void ShowScreensaverChooser(const QString &screen = QString());

    Q_SCRIPTABLE void EnableUIDebug(bool enable);
    Q_SCRIPTABLE void SetVisible(int screenNum,bool);
    Q_SCRIPTABLE void FixGeometry(int screenNum);
    Q_SCRIPTABLE void Reset();
    Q_SCRIPTABLE void PrintInfo();
    Q_SCRIPTABLE void Refresh(bool silent = true);
    Q_SCRIPTABLE QList<int> GetIconSize();
    Q_SCRIPTABLE int GetIconSizeMode();
    Q_SCRIPTABLE bool SetIconSizeMode(int);
protected:
    void showWallpaperSettings(QString name, int mode = 0);
private:
    explicit Desktop();
    ~Desktop();
    friend class DDEDesktop::Singleton<Desktop>;
    Q_DISABLE_COPY(Desktop)

    QScopedPointer<DesktopPrivate> d;
};

#endif // DESKTOP_H
