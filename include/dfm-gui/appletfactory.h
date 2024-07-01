// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETFACTORY_H
#define APPLETFACTORY_H

#include <dfm-gui/dfm_gui_global.h>

#include <QScopedPointer>

DFMGUI_BEGIN_NAMESPACE

class Applet;
class Containment;

class AppletFactoryData;
class AppletFactory
{
public:
    using CreateFunc = std::function<Applet *(const QString &id, Containment *parent, QString *error)>;
    static AppletFactory *instance();

    bool regCreator(const QString &id, CreateFunc creator, QString *errorString = nullptr);
    Applet *create(const QString &id, Containment *parent = nullptr, QString *errorString = nullptr);

    QUrl pluginComponent(const QString &plugin, const QString &qmlFile, QString *errorString = nullptr) const;

private:
    explicit AppletFactory();
    QScopedPointer<AppletFactoryData> d;
};

class ViewAppletFacotryData;
class ViewAppletFactory
{
public:
    using CreateFunc = std::function<Applet *(const QUrl &url, Containment *parent, QString *error)>;
    static ViewAppletFactory *instance();

    bool regCreator(const QString &plugin, const QString &qmlFile, const QString &scheme,
                    CreateFunc creator, QString *errorString = nullptr);
    Applet *create(const QUrl &url, Containment *parent = nullptr, QString *errorString = nullptr);

private:
    explicit ViewAppletFactory();
    QScopedPointer<ViewAppletFacotryData> d;
};

DFMGUI_END_NAMESPACE

#endif   // APPLETFACTORY_H
