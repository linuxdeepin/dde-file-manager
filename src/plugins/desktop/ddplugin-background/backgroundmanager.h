/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "ddplugin_background_global.h"

#include "interfaces/background/abstractbackground.h"

#include <QObject>

DDP_BACKGROUND_BEGIN_NAMESPACE

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
    virtual QMap<QString, DFMBASE_NAMESPACE::BackgroundWidgetPointer> allBackgroundWidgets();
    virtual DFMBASE_NAMESPACE::BackgroundWidgetPointer backgroundWidget(const QString &screen);
    virtual QMap<QString, QString> allBackgroundPath();
    virtual QString backgroundPath(const QString &screen);
    virtual void setBackgroundPath(const QString &screen,const QString &path);

private slots:
    void onBackgroundBuild();
    void onDetachWindows();
    void onRestBackgroundManager();
    void onGeometryChanged();
    void onWmDbusStarted(QString name, QString oldOwner, QString newOwner);
    void onWorkspaceSwitched(int from, int to);
    void onAppearanceCalueChanged(const QString& key);

private:
    void updateBackgroundPaths();
    void resetBackgroundImage();
    QString getBackground(const QString &screen);
    DFMBASE_NAMESPACE::BackgroundWidgetPointer createBackgroundWidget(QWidget *root);

private:
    BackgroundManagerPrivate *const d = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // DDP_BACKGROUND_END_NAMESPACE
