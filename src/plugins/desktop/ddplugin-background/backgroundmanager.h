// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include "ddplugin_background_global.h"
#include "backgroundservice.h"

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
    void onBackgroundChanged();

private:
    void updateBackgroundPaths();
    void resetBackgroundImage();
    DFMBASE_NAMESPACE::BackgroundWidgetPointer createBackgroundWidget(QWidget *root);

private:
    BackgroundManagerPrivate *const d = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // DDP_BACKGROUND_END_NAMESPACE
