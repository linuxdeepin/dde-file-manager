/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "dfm-base/widgets/screenglobal.h"
#include "dfm-base/widgets/abstractbackground.h"
#include "dfm_desktop_service_global.h"
#include <QObject>

DSB_D_BEGIN_NAMESPACE

class BackgroundManagerPrivate;

class BackgroundManager : public QObject
{
    Q_OBJECT
    friend class BackgroundManagerPrivate;
public:
    explicit BackgroundManager(QObject *parent = nullptr);
    ~BackgroundManager();
     void init();
public:
    virtual QMap<QString, dfmbase::BackgroundWidgetPointer> allBackgroundWidgets();
    virtual dfmbase::BackgroundWidgetPointer backgroundWidget(const QString &screen);
    virtual QMap<QString, QString> allBackgroundPath();
    virtual QString backgroundPath(const QString &screen);
    virtual void setBackgroundPath(const QString &screen,const QString &path);
signals:
    void sigBackgroundBuilded(int mode);

private slots:
    void onBackgroundBuild();
    void onRestBackgroundManager();
    void onScreenGeometryChanged();
    void onWmDbusStarted(QString name, QString oldOwner, QString newOwner);
    void onWorkspaceSwitched(int from, int to);
    void onAppearanceCalueChanged(const QString& key);

private:
    void updateBackgroundPaths();
    void resetBackgroundImage();
    QString getBackground(const QString &screen);
    dfmbase::BackgroundWidgetPointer createBackgroundWidget(dfmbase::ScreenPointer sp);

private:
    BackgroundManagerPrivate *const d = nullptr;
};

DSB_D_END_NAMESPACE

#endif // BACKGROUNDMANAGER_H
