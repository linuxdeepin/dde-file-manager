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
    using CreateFunc = std::function<Applet *(const QUrl &url, Containment *parent, QString *error)>;
    static AppletFactory *instance();

    bool regCreator(const QString &scheme, CreateFunc creator, QString *errorString = nullptr);
    Applet *create(const QUrl &url, Containment *parent = nullptr, QString *errorString = nullptr);

protected:
    explicit AppletFactory();
    QScopedPointer<AppletFactoryData> d;
};

class ViewAppletFactory : public AppletFactory
{
public:
    static ViewAppletFactory *instance();
    bool regCreator(const QString &plugin, const QString &qmlFile, const QString &scheme,
                    AppletFactory::CreateFunc creator, QString *errorString = nullptr);

private:
    explicit ViewAppletFactory();
};

DFMGUI_END_NAMESPACE

#endif   // APPLETFACTORY_H
