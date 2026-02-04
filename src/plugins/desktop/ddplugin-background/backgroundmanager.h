// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include "ddplugin_background_global.h"
#include "backgroundservice.h"

#include "backgrounddefault.h"

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
    virtual QMap<QString, BackgroundWidgetPointer> allBackgroundWidgets();
    virtual BackgroundWidgetPointer backgroundWidget(const QString &screen);
    virtual QMap<QString, QString> allBackgroundPath();
    virtual QString backgroundPath(const QString &screen);
    void restBackgroundManager();
    bool useColorBackground();

private slots:
    void onBackgroundBuild();
    void onDetachWindows();
    void onGeometryChanged();
    void onBackgroundChanged();

private:
    BackgroundWidgetPointer createBackgroundWidget(QWidget *root);
    void updateBackgroundWidgets();

private:
    BackgroundManagerPrivate *const d = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // DDP_BACKGROUND_END_NAMESPACE
